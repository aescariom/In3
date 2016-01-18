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
#include "../lib/output/rgb.h"
#include "../lib/output/lcd.h"

#include "../lib/input/switch.h"
#include "../lib/input/rotary.h"

#define output(directions,pin) (directions |= pin) // set port direction for output
#define input(directions,pin) (directions &= (~pin)) // set port direction for input
#define set(port,pin) (port |= pin) // set port pin
#define clear(port,pin) (port &= (~pin)) // clear port pin
#define pin_test(pins,pin) (pins & pin) // test for port pin
#define bit_test(byte,bit) (byte & (1 << bit)) // test for bit set

#define TWI_TIMEOUT 200

#define TWI_FREQ 100000L
#define CPU_FREQ 16000000L

volatile unsigned char auxTargetTemperature = 0;
volatile bool changeTargetTemp = false;
volatile unsigned char targetTemperature = 36;

volatile unsigned char menuOpt = 1;
volatile bool menuEnabled = false;
volatile bool temperatureMenu = false;

unsigned char temperature_addr = 0x28; 
unsigned char temperature = 0;
unsigned char humidity = 0;

char buff[6];

//Menu Strings
const char MENU[2][20] = {
    "%c <<Back>>",
    "%c Change temp"
};

RGB* rgb;
Switch* sw_rotary;
LCD* lcd;



void i2c_init_master(){
    // SCL freq = F_CPU/(16+2(TWBR)*prescalerValue)
	TWBR = 72;//((CPU_FREQ / TWI_FREQ) - 16) / 2;
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

void init(){
    sw_rotary = new Switch(&PORTD, &DDRD, &PIND, PD4);
    sw_rotary->init();
    rgb = new RGB(&PORTB, &DDRB, &PINB, PB2, PB1, PB5);
    lcd = new LCD();
    lcd->setDb7(&PORTC, &DDRC, PC3);
    lcd->setDb6(&PORTC, &DDRC, PC2);
    lcd->setDb5(&PORTC, &DDRC, PC1);
    lcd->setDb4(&PORTC, &DDRC, PC0);
    lcd->setE(&PORTD, &DDRD, PD6);
    lcd->setRs(&PORTD, &DDRD, PD7);
    
    lcd->init();
    lcd->erase();
    lcd->cursorOff();
    rgb->init();
    initRotary();
    
    usart_init(R_UBRR);
   	i2c_init_master();
   	sei();
}

void freeAll(){
    delete(sw_rotary);
    delete(rgb);
    delete(lcd);
}

void printVars(){
    char line[16];
    sprintf(line, "Temp.: %2i/%2i%cC", temperature, targetTemperature, 0b11011111);
    lcd->println(line, 1);
    sprintf(line, "Humidity: %3i%%", humidity);
    lcd->println(line, 2);
}

void printMenu(){
    char line[16];
    sprintf(line, MENU[0], menuOpt == 0 ? '>' : ' ');
    lcd->println(line, 1);
    sprintf(line, MENU[1], menuOpt == 1 ? '>' : ' ');
    lcd->println(line, 2);
}

void printChangingTargetTemperature(){
    char line[16];
    sprintf(line, "%2i%cC", auxTargetTemperature, 0b11011111);
    lcd->println(line, 2);
}

void printTemperatureMenu(){
    lcd->println("Set new value:", 1);
    auxTargetTemperature = targetTemperature;
    printChangingTargetTemperature();
}

void menuAction(volatile unsigned char opt){
    switch(opt){
        case 0:
            lcd->erase();
            printVars();
            menuEnabled = false;
            temperatureMenu = false;
            break;
        case 1:
            lcd->erase();
            printTemperatureMenu();
            menuEnabled = false;
            temperatureMenu = true;
            break;
    }
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
		targetTemperature=buff[4];
	}
}

int main() {
    long lastEncoderValue = 0;
    char lastTarget = 0;
    
    init();

    while (1) {
        if(menuEnabled){
            if(lastEncoderValue < encoderValue){
                ++menuOpt;
            }else if(lastEncoderValue > encoderValue){
                --menuOpt;
            }
            if(lastEncoderValue != encoderValue){
                menuOpt %= 2;
                printMenu();
                lastEncoderValue = encoderValue;
            }
            if(sw_rotary->isPressed()){
                menuAction(menuOpt);
            }
        }else if(temperatureMenu){
            
            if(lastEncoderValue < encoderValue){
                ++auxTargetTemperature;
            }else if(lastEncoderValue > encoderValue){
                --auxTargetTemperature;
            }
            if(auxTargetTemperature == 255){
                auxTargetTemperature = 0;
            }else if(auxTargetTemperature > 50){
                auxTargetTemperature = 50;
            }
            
            if(lastEncoderValue != encoderValue){
                printChangingTargetTemperature();
                lastEncoderValue = encoderValue;
            }
            if(sw_rotary->isPressed()){
                targetTemperature = auxTargetTemperature;
                changeTargetTemp = true;
                lcd->erase();
                printVars();
                menuEnabled = false;
                temperatureMenu = false;
            }
        }else{
            if(sw_rotary->isPressed()){
                menuOpt = 0;
                lcd->erase();
                printMenu();
                menuEnabled = true;
                temperatureMenu = false;
                lastEncoderValue = encoderValue;
            }else{
                printVars();
            }
        }
        if(temperature > targetTemperature){
            rgb->turnOnBlue();
        }else if(temperature == targetTemperature){
            rgb->turnOnGreen();
        }else{
            rgb->turnOnRed();
        }

   		temperature = readTemperature();
   		humidity = readHumidity();
   	
   		i2c_start();
    	i2c_read_address(temperature_addr);
	  	i2c_write_data('S');
      	i2c_write_data(targetTemperature);
      	i2c_stop();
    }
    
    freeAll();
}