#include "relay.h"

Relay::Relay(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask){
	this->port = port;
	this->dir = dir;
	this->mask = mask;
}

void Relay::init(){
	*(this->dir) |= 1 << this->mask; // output pin
}

void Relay::open(){
    *(this->port) |= (1 << this->mask);
}

void Relay::close(){
    *(this->port) &= ~(1 << this->mask);
}