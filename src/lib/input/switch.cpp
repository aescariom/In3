#include "switch.h"

Switch::Switch(volatile uint8_t *port, volatile uint8_t *dir, volatile uint8_t *pin, uint8_t mask){
	this->port = port;
	this->pin = pin;
	this->dir = dir;
	this->mask = mask;
}

void Switch::init(){
	*(this->port) |= (1 << this->mask); // turn on pull-up
	*(this->dir) &= ~(1 << this->mask); // input pin
}

bool Switch::isPressed(){
	return (*(this->pin) & (1 << this->mask)) == 0; // pin test
}