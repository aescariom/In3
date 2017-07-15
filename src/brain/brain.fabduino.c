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
#include <inttypes.h>

#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "../lib/network/serial.h"
#include "../lib/output/lcdi2c.h"
#include "../lib/output/fan.h"
#include "../lib/output/led.h"
#include "../lib/output/relay.h"
#include "../lib/output/buzzer.h"
#include "../lib/ext/i2chw/i2cmaster.h"
#include "../lib/ext/dht22/dht22.h"
#include "../lib/input/analogSwitch.h"
#include "../lib/input/rotary.h"

#define TWI_TIMEOUT 200
#define DHT_TIMEOUT 200

int errorCount = 0;
float temperature = 0;
float humidity = 0;
int readErrors = 0;
int lastRead = 0;
unsigned char tempFlag = ' ';
unsigned char humFlag = ' ';
volatile unsigned char targetTemperature = 36;

volatile unsigned char auxTargetTemperature = 36;
volatile bool changeTargetTemp = false;

char buff[6];

volatile unsigned char menuOpt = 1;
volatile bool menuEnabled = false;
volatile bool temperatureMenu = false;

struct dht22 *dht;

//Menu Strings
const char MENU[2][20] = {
    "%c <<Back>>",
    "%c Change temp"
};

void getTempAndHum();

Led* led;
Buzzer* buzzer;
LCD_I2C* lcd;
AnalogSwitch* sw_rotary;
Fan* fan1;
Fan* fan2;
Relay* relay1;
Relay* relay2;

int timeToRead = 1;

int tot_overflow = 0;
ISR(TIMER1_OVF_vect)
{
    // keep a track of number of overflows
    tot_overflow++;
  
    // check for number of overflows here itself
    // 31 overflows = 1 seconds delay (approx.)
    if (tot_overflow >= 31) // NOTE: '>=' used instead of '=='
    {
        led->toggle();
        timeToRead = 1;
        // no timer reset required here as the timer
        // is reset every time it overflows
  
        tot_overflow = 0;   // reset overflow counter
    }
}

void timer1_init()
{
    // set up timer with prescaler = 8
	TCCR1B |= (1 << CS11);
    // initialize counter
    TCNT1 = 0;
    // enable overflow interrupt
    TIMSK1 |= (1 << TOIE1);
  
    // initialize overflow counter variable
    tot_overflow = 0;
}

void turnOnFans(){
  OCR0A=255; // 12V
  OCR0B=255; // 12V
}

void turnOffFanB(){
  OCR0B=0;
}

void turnOffFanA(){
  OCR0A=0;
}

void turnOffFans(){
  OCR0A=0;
  OCR0B=0;
}

void init(){
	i2c_init();
	
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

	led = new Led(&PORTD, &DDRD, PD4);
    
    led->init();
	led->toggle();
	
	timer1_init();
    
    lcd = new LCD_I2C(0x27, 16, 2);
    lcd->backlight(1);    
	lcd->init();
	
	/*buzzer = new Buzzer(&PORTD, &DDRD, PD0);
    buzzer->init();*/
    
    sw_rotary = new AnalogSwitch(0x07, 512);
    sw_rotary->init();
    
	fan1 = new Fan(&PORTD, &DDRD, PD6);
	fan2 = new Fan(&PORTD, &DDRD, PD5);
	
	fan1->init();
	fan2->init();
	turnOffFans();
	
    relay1 = new Relay(&PORTD, &DDRD, PD7);
    relay2 = new Relay(&PORTB, &DDRB, PB0);
	relay1->init();
	relay2->init();
	
    initRotary();
}

void printVars(){
    char line[16];
    sprintf(line, "%cTemp.:  %2i/%2i%cC", tempFlag, (int)temperature, targetTemperature, 0b11011111);
	lcd->println(line, 0);
    sprintf(line, "%cHumidity:  %3i%%", humFlag, (int)humidity);
	lcd->println(line, 1);
}

void printMenu(){
    char line[16];
    sprintf(line, MENU[0], menuOpt == 0 ? '>' : ' ');
    lcd->println(line, 0);
    sprintf(line, MENU[1], menuOpt == 1 ? '>' : ' ');
    lcd->println(line, 1);
}

void printChangingTargetTemperature(){
    char line[16];
    sprintf(line, "%2i%cC", auxTargetTemperature, 0b11011111);
    lcd->println(line, 1);
}

void printTemperatureMenu(){
    char line[16];
    sprintf(line, "Set new value:");
    lcd->println(line, 0);
    auxTargetTemperature = targetTemperature;
    printChangingTargetTemperature();
}

void menuAction(volatile unsigned char opt){
    switch(opt){
        case 0:
            lcd->clear();
            printVars();
            menuEnabled = false;
            temperatureMenu = false;
            break;
        case 1:
            lcd->clear();
            printTemperatureMenu();
            menuEnabled = false;
            temperatureMenu = true;
            break;
    }
}

void heat(){
    relay1->open();
    relay2->close();
}

void cool(){
	relay2->close();
   	relay1->open();
}

void none(){
	relay2->close();
   	relay1->close();
}

void getTempAndHum(){
	float temperature1, humidity1;
	float temperature2, humidity2;
	tempFlag = humFlag = ' ';
	if(dht_gettemperaturehumidity(&temperature1, &humidity1, &PORTC, &PINC, &DDRC, PC0) != 0){
		temperature1 = -1;
		humidity1 = -1;
		humFlag = tempFlag = '?';
	}
	if(dht_gettemperaturehumidity(&temperature2, &humidity2, &PORTC, &PINC, &DDRC, PC1) != 0){
		temperature2 = -1;
		humidity2 = -1;
		humFlag = tempFlag = '?';
	}
	if(temperature1 < 0 && temperature2 < 0){
		errorCount++;
		if(errorCount > 10){
			temperature = -1;
		}
	}else if(temperature1 < 0){
		temperature = temperature2;
		errorCount = 0;
	}else if(temperature2 < 0){
		temperature = temperature1;
		errorCount = 0;
	}else{
		temperature = (temperature1 + temperature2) / 2;
		errorCount = 0;
	}
	if(tempFlag != '?'){
		int diff = temperature1 - temperature2;
		if(diff > 3 || diff < -3){
			tempFlag = '!';
		}
	}
	if(humidity1 < 0 && humidity2 < 0){
		humidity = -1;
	}else if(humidity1 < 0){
		humidity = humidity2;
	}else if(humidity2 < 0){
		humidity = humidity1;
	}else{
		humidity = (humidity1 + humidity2) / 2;
	}

	if(humFlag != '?'){
		int diff = humidity1 - humidity2;
		if(diff > 10 || diff < -10){
			humFlag = '!';
		}
	}
}

int main() {
    init();
    
    long lastEncoderValue = 0;
	unsigned long cycle = 0;

    while (1) {
    	if(timeToRead > 0){
    		timeToRead = 0;		
			getTempAndHum();
			
			if(temperature < 0){
				none();
				turnOffFans();
			}else if(targetTemperature > temperature){
				turnOnFans();
				heat();
			}else if(targetTemperature < temperature){
				turnOnFans();
				cool();
			}else{
				none();
				turnOffFans();
			}
    	}
    
        if(menuEnabled){
    		cycle += 1;
	    	cycle = cycle%400000;
    		if(cycle == 0){
            	menuEnabled = false;
	            temperatureMenu = false;
    		}
            if(lastEncoderValue < encoderValue){
            	cycle = 0;
                ++menuOpt;
            }else if(lastEncoderValue > encoderValue){
            	cycle = 0;
                --menuOpt;
            }
            if(lastEncoderValue != encoderValue){
                menuOpt %= 2;
                printMenu();
                lastEncoderValue = encoderValue;
            }
            if(sw_rotary->isPressed()){
                menuAction(menuOpt);
        		buzzer->beep();
        		_delay_ms(250);
            }
        }else if(temperatureMenu){
    		cycle += 1;
	    	cycle = cycle%400000;
    		if(cycle == 0){
            	menuEnabled = false;
	            temperatureMenu = false;
    		}
            if(cycle > 1000){
                if(lastEncoderValue < encoderValue){
                    cycle = 0;
                    if(auxTargetTemperature < 40){
                        ++auxTargetTemperature;
                    }
                }else if(lastEncoderValue > encoderValue){
                    cycle = 0;
                    if(auxTargetTemperature > 30){
                        --auxTargetTemperature;
                    }
                }
            }
            
            if(lastEncoderValue != encoderValue){
                printChangingTargetTemperature();
                lastEncoderValue = encoderValue;
            }
            if(sw_rotary->isPressed()){
        		buzzer->beep();
                targetTemperature = auxTargetTemperature;
                changeTargetTemp = true;
                lcd->clear();
                printVars();
                menuEnabled = false;
                temperatureMenu = false;
        		_delay_ms(250);
            }
        }else{
            if(sw_rotary->isPressed()){
                menuOpt = 0;
                lcd->clear();
                printMenu();
                menuEnabled = true;
                temperatureMenu = false;
                lastEncoderValue = encoderValue;
        		buzzer->beep();
        		_delay_ms(250);
            }else{
                printVars();
            }
        }
	}
}
