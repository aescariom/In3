/*
DHT Library 0x03

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.

References:
  - DHT-11 Library, by Charalampos Andrianakis on 18/12/11
*/


#ifndef DHT_H_
#define DHT_H_

#include <stdio.h>
#include <avr/io.h>

//timeout retries
#define DHT_TIMEOUT 200

//functions
extern int8_t dht_gettemperaturehumidity(float *temperature, float *humidity, volatile uint8_t *port, volatile uint8_t *pin, volatile uint8_t *dir, uint8_t mask);

#endif
