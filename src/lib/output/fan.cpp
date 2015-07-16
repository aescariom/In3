#include "fan.h"

Fan::Fan(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask){
	this->port = port;
	this->dir = dir;
	this->mask = mask;
}

void Fan::init(){
	*(this->dir) |= 1 << this->mask; // output pin
}

void Fan::turnOn(){
    *(this->port) |= (1 << this->mask);
}

void Fan::turnOff(){
    *(this->port) &= ~(1 << this->mask);
}