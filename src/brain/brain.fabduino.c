/********************************************************
*
*  brain.fabduino.c
* 
*  Alejandro Escario Méndez
*   03/05/2015  
*  MIT license
*********************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>

#include "../lib/network/serial.h"

#define output(directions,pin) (directions |= pin) // set port direction for output
#define input(directions,pin) (directions &= (~pin)) // set port direction for input
#define set(port,pin) (port |= pin) // set port pin
#define clear(port,pin) (port &= (~pin)) // clear port pin
#define pin_test(pins,pin) (pins & pin) // test for port pin
#define bit_test(byte,bit) (byte & (1 << bit)) // test for bit set

#define TWI_TIMEOUT 200


#define LED_DIRECTION DDRB
#define LED_PORT PORTB
#define LED_PIN (1 << PB5)

unsigned char controlpanel_addr = 0x28; 
unsigned char temperature_addr = 0x20; 
unsigned char temperature = 0;
unsigned char targetTemp = 36;
unsigned char humidity = 0;

char buff[6];

void i2c_init_master(){
    // SCL freq = F_CPU/(16+2(TWBR)*prescalerValue)
    TWBR = 32;  // Bit rate
    TWSR = (0 << TWPS1) | (0 << TWPS0); // Setting prescalar bits
}

void i2c_waitforTWSR(int var){
	int ticks = 0;
	while((TWSR & 0xF8)!= var && ticks < TWI_TIMEOUT){
		++ticks;
	}
}

void i2c_waitforTWCR(int var){
	int ticks = 0;
	while(!(TWCR & (1 << var)) && ticks < TWI_TIMEOUT){
		++ticks;
	}
}

void i2c_start(){
   TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN); // put start condition
   i2c_waitforTWCR(TWINT); // wait until start condition is transmitted
   i2c_waitforTWSR(TW_START); // wait for ACK
}

void i2c_repeated_start(){
   TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN); // put start condition
   i2c_waitforTWCR(TWINT); // wait until start condition is transmitted
   i2c_waitforTWSR(TW_REP_START); // wait for ACK  
}

void i2c_stop(){
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);  // put stop condition
    i2c_waitforTWCR(TWSTO);  // wait until stop condition is transmitted
}

void i2c_write_address(unsigned char addr)
{
    TWDR = addr;  // write addr to "call"
    TWCR = (1 << TWINT) | (1 << TWEN);    // Enable TWI and clear interrupt flag
    i2c_waitforTWCR(TWINT); // Wait until TWDR byte is transmitted
    i2c_waitforTWSR(TW_MT_SLA_ACK);  // Check for the acknowledgement
}

void i2c_write_data(unsigned char data)
{
    TWDR = data;  // write data in TWDR
    TWCR = (1 << TWINT) | (1 << TWEN);    // clc TWI interrupt flag,En TWI
    i2c_waitforTWCR(TWINT); // Wait until TWDR byte transmitted
   	i2c_waitforTWSR(TW_MT_DATA_ACK); // Check for the acknowledgement
}

void i2c_read_address(unsigned char data)
{
    TWDR=data;  // Address and read instruction
    TWCR=(1<<TWINT)|(1<<TWEN);    // clc TWI interrupt flag,En TWI
    i2c_waitforTWCR(TWINT); // Wait until TWDR byte transmitted
    i2c_waitforTWSR(TW_MR_SLA_ACK);  // Check for the acknowledgement
}


unsigned char i2c_read_data()
{
    TWCR=(1<<TWINT)|(1<<TWEN);    // clc TWI interrupt flag,En TWI
    i2c_waitforTWCR(TWINT); // Wait until TWDR byte transmitted
    i2c_waitforTWSR(TW_MR_DATA_NACK); // Check for the acknowledgement

	unsigned char ret = TWDR;
	
    return ret;
}

unsigned char readTemperature(){
	unsigned char temp;
	i2c_start();
    i2c_write_address(temperature_addr);
    i2c_write_data('T');
    i2c_stop();
    
	i2c_start();
    i2c_read_address(temperature_addr+1);
    temp = i2c_read_data();
    i2c_stop();
    
    return temp;
}

unsigned char readHumidity(){
	unsigned char hum;
	i2c_start();
    i2c_write_address(temperature_addr);
    i2c_write_data('H');
    i2c_stop();
    
	i2c_start();
    i2c_read_address(temperature_addr+1);
    hum = i2c_read_data();
    i2c_stop();
    
    return hum;
}

void sendInt(char tag, int val){
    usart_putchar(1);
    usart_putchar(2);
    usart_putchar(3);
    usart_putchar(4);
    usart_putchar(tag);
    usart_putchar(val);
    usart_putchar(0);
}

ISR (USART_RX_vect) {
	buff[0] = buff[1];
	buff[1] = buff[2];
	buff[2] = buff[3];
	buff[3] = buff[4];
	buff[4] = buff[5];
	buff[5] = UDR0;
	if(buff[0] == 1 && buff[1] == 2 && buff[2] == 3 && buff[3] == 4){
		targetTemp=buff[4];
	}
}

int main() {
	
   output(LED_DIRECTION, LED_PIN);

	char lastTarget = 0;
	char line[16];
	usart_init(R_UBRR);
   	i2c_init_master();
   	sei();
	int i = 0;
   	while (1) {
      	_delay_ms(1000);
   		temperature = readTemperature();
   		humidity = readHumidity();
   	
   		i2c_start();
    	i2c_read_address(temperature_addr);
	  	i2c_write_data('S');
      	i2c_write_data(targetTemp);
      	i2c_stop();
	  

      	i2c_start();
      	i2c_read_address(controlpanel_addr+1);
      	unsigned char temp = i2c_read_data();
      	if(temp > 0){
      		targetTemp = temp;
      	}
      	i2c_stop();
      
      	i2c_start();
      	i2c_write_address(controlpanel_addr);
      	i2c_write_data('T');
      	i2c_write_data(temperature);
      	i2c_write_data('H');
      	i2c_write_data(humidity);
	  	i2c_write_data('S');
      	i2c_write_data(targetTemp);
      	i2c_stop();
      
      	sendInt('T', temperature);
		sendInt('H', humidity);
    	if(lastTarget != targetTemp){
    		sendInt('O', targetTemp);
    		lastTarget = targetTemp;
    	}
   }
}