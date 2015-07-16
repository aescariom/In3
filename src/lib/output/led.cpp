#include "led.h"

Led::Led(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask){
	this->port = port;
	this->dir = dir;
	this->mask = mask;
}

void Led::init(){
	*(this->dir) |= 1 << this->mask; // output pin
}

void Led::blink(){
	this->toggle();
    _delay_ms(_O_LED_DELAY_MS);
	this->toggle();
}

void Led::toggle(){
    *(this->port) ^= (1 << this->mask);
}

void Led::turnOn(){
    *(this->port) |= (1 << this->mask);
}

void Led::turnOff(){
    *(this->port) &= ~(1 << this->mask);
}