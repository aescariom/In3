/********************************************************
*
*  controlpanel.fabduino.c
* 
*  Alejandro Escario Méndez
*   03/05/2015  
*
*  MIT license
*********************************************************/

#include <avr/io.h> // AVR ctes

#include <util/delay.h>
#include <stdio.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "../lib/output/led.h"
#include "../lib/output/rgb.h"
#include "../lib/output/lcd.h"

#include "../lib/input/switch.h"
#include "../lib/input/rotary.h"

#include "../lib/network/i2c/i2cSlave.h"

LCD* lcd;
Led* led_sck;
Switch* sw_rotary;
RGB* rgb;

volatile unsigned char auxTargetTemperature = 0;
volatile unsigned char targetTemperature = 36;
volatile bool changeTargetTemp = false;
volatile unsigned char temperature = 0;
volatile unsigned char humidity = 0;

unsigned char my_addr = 0x28;

volatile unsigned char menuOpt = 1;
volatile bool menuEnabled = false;
volatile bool temperatureMenu = false;

//Menu Strings
const char MENU[2][20] = {
							"%c <<Back>>",
							"%c Change temp"
						};

void init(){
	led_sck = new Led(&PORTB, &DDRB, PB5);
	sw_rotary = new Switch(&PORTD, &DDRD, &PIND, PD4);
	rgb = new RGB(&PORTC, &DDRC, &PINC, PC1, PC0, PC2);
	lcd = new LCD();
	lcd->setDb7(&PORTB, &DDRB, PB2);
	lcd->setDb6(&PORTB, &DDRB, PB1);
	lcd->setDb5(&PORTB, &DDRB, PB0);
	lcd->setDb4(&PORTD, &DDRD, PD7);
	lcd->setE(&PORTD, &DDRD, PD6);
	lcd->setRs(&PORTD, &DDRD, PD5);
	
	led_sck->init();
	sw_rotary->init();
	rgb->init();
	lcd->init();
	lcd->erase();
	lcd->cursorOff();
	initRotary();
	i2c_init_slave();
}

void freeAll(){
	delete(led_sck);
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

int main(void) {
	long lastEncoderValue = 0;
	
	init();
	
	while(1){
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
		
	}
	
	freeAll();
}