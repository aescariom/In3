/*
DHT Library 0x03

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>

#include "dht22.h"

/*
 * get data from sensor
 */
int8_t dht_getdata(float *temperature, float *humidity, volatile uint8_t *port, volatile uint8_t *pin, volatile uint8_t *dir, uint8_t mask) {
	uint8_t bits[5];
	uint8_t i,j = 0;

	memset(bits, 0, sizeof(bits));

	//reset port
	*dir |= (1<<mask); //output
	*port |= (1<<mask); //high
	_delay_ms(100);

	//send request
	*port &= ~(1<<mask); //low
	_delay_us(600);
	*port |= (1<<mask); //high
	*dir &= ~(1<<mask); //input
	_delay_us(40);

	//check start condition 1
	if((*pin & (1<<mask))) {
		return -1;
	}
	_delay_us(80);
	//check start condition 2
	if(!(*pin & (1<<mask))) {
		return -1;
	}
	_delay_us(80);

	//read the data
	uint16_t timeoutcounter = 0;
	for (j=0; j<5; j++) { //read 5 byte
		uint8_t result=0;
		for(i=0; i<8; i++) {//read every bit
			timeoutcounter = 0;
			while(!(*pin & (1<<mask))) { //wait for an high input (non blocking)
				timeoutcounter++;
				if(timeoutcounter > DHT_TIMEOUT) {
					return -1; //timeout
				}
			}
			_delay_us(30);
			if(*pin & (1<<mask)) //if input is high after 30 us, get result
				result |= (1<<(7-i));
			timeoutcounter = 0;
			while(*pin & (1<<mask)) { //wait until input get low (non blocking)
				timeoutcounter++;
				if(timeoutcounter > DHT_TIMEOUT) {
					return -1; //timeout
				}
			}
		}
		bits[j] = result;
	}

	//reset port
	*dir |= (1<<mask); //output
	*port |= (1<<mask); //low

	//check checksum
	if ((uint8_t)(bits[0] + bits[1] + bits[2] + bits[3]) == bits[4]) {
		//return temperature and humidity
		uint16_t rawhumidity = bits[0]<<8 | bits[1];
		uint16_t rawtemperature = bits[2]<<8 | bits[3];
		if(rawtemperature & 0x8000) {
			*temperature = (float)((rawtemperature & 0x7FFF) / 10.0) * -1.0;
		} else {
			*temperature = (float)(rawtemperature)/10.0;
		}
		*humidity = (float)(rawhumidity)/10.0;
		return 0;
	}

	return -1;
}

/*
 * get temperature and humidity
 */
int8_t dht_gettemperaturehumidity(float *temperature, float *humidity, volatile uint8_t *port, volatile uint8_t *pin, volatile uint8_t *dir, uint8_t mask) {
	return dht_getdata(temperature, humidity, port, pin, dir, mask);
}


