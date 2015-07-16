/********************************************************
*
*  i2cslave.fabduino.c
* 
*  Alejandro Escario Méndez
*   03/05/2015  
* 
*  (c) Massachusetts Institute of Technology 2015
*  Permission granted for experimental and personal use;
*  license for commercial sale available from MIT.
*********************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "../lib/output/fan.h"
#include "../lib/output/relay.h"
#include "../lib/operators/new.h"

#define output(directions,pin) (directions |= pin) // set port direction for output
#define input(directions,pin) (directions &= (~pin)) // set port direction for input
#define set(port,pin) (port |= pin) // set port pin
#define clear(port,pin) (port &= (~pin)) // clear port pin
#define pin_test(pins,pin) (pins & pin) // test for port pin
#define bit_test(byte,bit) (byte & (1 << bit)) // test for bit set


#define DHT_DIR DDRD
#define DHT_PORT PORTD
#define DHT_SENS PD3 // PD4
#define DHT_PIN PIND

#define PUMP PD0
#define PUMP_DIRECTION DDRD
#define PUMP_PORT PORTD



#define LED_DIRECTION DDRB
#define LED_PORT PORTB
#define LED_PIN (1 << PB5)

#define DHT_TIMEOUT 200

unsigned char my_addr = 0x20;
unsigned char regaddr, regdata;

Fan* fan1;
Fan* fan2;
Relay* relay1;
Relay* relay2;

volatile int8_t temperature, humidity, targetTemp;

void i2c_init_slave(){
    TWAR = my_addr;
    TWDR = 0x00;
    // SCL freq = F_CPU/(16+2(TWBR)*prescalerValue)
    TWBR = 32;  // Bit rate
    TWSR = (0 << TWPS1) | (0 << TWPS0); // Setting prescalar bits
    TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN) | (1<<TWIE);    // get ACK, en TWI, clear int flag
}

int8_t dht_gettemperaturehumidity(int8_t *temperature, int8_t *humidity, volatile uint8_t *port, volatile uint8_t *pin, volatile uint8_t *dir, uint8_t mask) {
	uint8_t bits[5] = {0};
	uint8_t i,j = 0;

	*dir |= (1<<mask);
	*port |= (1<<mask);
	_delay_ms(100);
	*port &= ~(1<<mask);
	_delay_ms(18);
	*port |= (1<<mask);
	*dir &= ~(1<<mask);
	_delay_us(40);
	if((*pin & (1<<mask))) {
		return -1;
	}
	_delay_us(80);
	if(!(*pin & (1<<mask))) {
		return -1;
	}
	_delay_us(80);
	uint16_t timeoutcounter = 0;
	for (j=0; j<5; j++) {
		uint8_t result=0;
		for(i=0; i<8; i++) {
			timeoutcounter = 0;
			while(!(*pin & (1<<mask))) {
				timeoutcounter++;
				if(timeoutcounter > DHT_TIMEOUT) {
					return -1;
				}
			}
			_delay_us(30);
			if(*pin & (1<<mask))
				result |= (1<<(7-i));
			timeoutcounter = 0;
			while(*pin & (1<<mask)) {
				timeoutcounter++;
				if(timeoutcounter > DHT_TIMEOUT) {
					return -1;
				}
			}
		}
		bits[j] = result;
	}
	
	if ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) == bits[4]) {
		*temperature = bits[2];
		*humidity = bits[0];
		return 0;
	}

	return -1;
}

ISR(TWI_vect){
  static unsigned char state = 0, lastVal = 0;
  // Disable Global Interrupt
  cli();
  switch(TW_STATUS){
    case TW_START:  // 0x08
    case TW_REP_START: // 0x10
    case TW_MT_SLA_ACK: // 0x18
    case TW_MT_SLA_NACK: // 0x20
    case TW_MT_DATA_ACK: // 0x28
    case TW_MT_DATA_NACK: // 0x30
    case TW_MR_SLA_ACK:   // 0x40
    case TW_MR_SLA_NACK:   // 0x48
    case TW_MR_DATA_ACK:  // 0x50
    case TW_MR_DATA_NACK:  //  0x58
    case TW_ST_ARB_LOST_SLA_ACK:  //  0xB0
    case TW_SR_ARB_LOST_SLA_ACK: //  0x68
    case TW_SR_GCALL_ACK:    //0x70
    case TW_SR_ARB_LOST_GCALL_ACK:  //  0x78
    case TW_SR_GCALL_DATA_ACK:   // 0x90
    case TW_SR_GCALL_DATA_NACK:  //  0x98
    case TW_NO_INFO:   // 0xF8
      TWCR |= (1<<TWINT);    // Clear TWINT Flag
      break;
    case TW_SR_SLA_ACK:      // 0x60: SLA+W received, ACK returned
      TWCR |= (1<<TWINT);    // Clear TWINT Flag
      break;
    case TW_SR_DATA_ACK:     // data received
      state = TWDR;
      if(lastVal == 'S'){
      	targetTemp = state;
      	state = 0;
      }
      lastVal = state;
      TWCR |= (1<<TWINT);    // Clear TWINT Flag
      break;
    case TW_SR_STOP:         // 0xA0: stop or repeated start condition received while selected
      TWCR |= (1<<TWINT);    // Clear TWINT Flag
      break;
    case TW_ST_SLA_ACK:      // 0xA8: SLA+R received, ACK returned
    	if(state == 'T'){
      		TWDR = temperature;           // Fill TWDR register whith the data to be sent 
      	}else if(state == 'H'){
      		TWDR = humidity;
      	}else{
      		TWDR = -1;
      	}
      TWCR = ((1 << TWEA) | (1 << TWINT) | (1 << TWEN) | (1 << TWIE));   // Enable TWI, Clear TWI interrupt flag
      break;
    case TW_ST_DATA_ACK:     // 0xB8: data transmitted, ACK received
      TWCR |= (1<<TWINT);    // Clear TWINT Flag
      break;
    case TW_ST_DATA_NACK:    // 0xC0: data transmitted, NACK received
      TWCR |= (1<<TWINT);    // Clear TWINT Flag
      break;
    case TW_ST_LAST_DATA:    // 0xC8: last data byte transmitted, ACK received
    case TW_BUS_ERROR:       // 0x00: illegal start or stop condition
    default:
      TWCR |= (1<<TWINT);    // Clear TWINT Flag
      break;
  }

  sei();
}

void turnOnFans(){
  OCR0A=255;
  OCR0B=127;
}

void turnOffFans(){
  OCR0A=0;
  OCR0B=0;
}

void init(){
	fan1 = new Fan(&PORTD, &DDRD, PD6);
	fan2 = new Fan(&PORTD, &DDRD, PD5);
	relay1 = new Relay(&PORTD, &DDRD, PD1);
	relay2 = new Relay(&PORTD, &DDRD, PD2);
	
	fan1->init();
	fan2->init();
	relay1->init();
	relay2->init();
}

void heat(){
	relay1->open();
   	relay2->close();
}

void cool(){
	relay2->open();
   	relay1->close();
}

int main() {
   //output(LED_DIRECTION, LED_PIN);
	int8_t temperature1, humidity1;
	int8_t temperature2, humidity2;

	init();

   	i2c_init_slave();
   	sei();
   	
   	DDRD   |= (1 << PD5);
   	DDRD   |= (1 << PD6);
   	
  PORTD = 0x00;
  // Initial TIMER0 Fast PWM
  // Fast PWM Frequency = fclk / (N * 256), Where N is the Prescaler
  // f_PWM = 11059200 / (64 * 256) = 675 Hz
  TCCR0A = 0b10100011; // Fast PWM 8 Bit, Clear OCA0/OCB0 on Compare Match, Set on TOP
  TCCR0B = 0b00000011; // Used 64 Prescaler
  TCNT0 = 0;           // Reset TCNT0
  OCR0A = 0;           // Initial the Output Compare register A & B
  OCR0B = 0;
  OCR0A=0;	// Initial Duty Cycle for Channel A
  OCR0B=0;	// Initial Duty Cycle for Channel B

int i = 0;
   	while (1){
   		if(dht_gettemperaturehumidity(&temperature1, &humidity1, &PORTD, &PIND, &DDRD, PD3) != 0){
   			temperature1 = -1;
   			humidity1 = -1;
   		}
   		if(dht_gettemperaturehumidity(&temperature2, &humidity2, &PORTD, &PIND, &DDRD, PD4) != 0){
   			temperature2 = -1;
   			humidity2 = -1;
   		}
   		if(temperature1 == -1 && temperature2 == -1){
   			temperature = 0;
   		}else if(temperature1 == -1){
   			temperature = temperature2;
   		}else if(temperature2 == -1){
   			temperature = temperature1;
   		}else{
   			temperature = (temperature1 + temperature2) / 2;
   		}
   		if(humidity1 == -1 && humidity2 == -1){
   			humidity = 0;
   		}else if(humidity1 == -1){
   			humidity = humidity2;
   		}else if(humidity2 == -1){
   			humidity = humidity1;
   		}else{
   			humidity = (humidity1 + humidity2) / 2;
   		}
   		
   		if(targetTemp > temperature){
	     	turnOnFans();
	     	heat();
     	}else if(targetTemp < temperature){
	     	turnOnFans();
	     	cool();
     	}else{
     		turnOffFans();
     	}
   		_delay_ms(1000);
   		
   }
}