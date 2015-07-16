#include "rgb.h"

RGB::RGB(volatile uint8_t *port, volatile uint8_t *dir, volatile uint8_t *pin, uint8_t red, 
    		uint8_t green, uint8_t blue){
	this->port = port;
	this->dir = dir;
	this->pin = pin;
	this->red = red;
	this->green = green;
	this->blue = blue;
}

void RGB::init(){
	*(this->dir) |= 1 << this->green; // output pin
	*(this->dir) |= 1 << this->red; // output pin
	*(this->dir) |= 1 << this->blue; // output pin
	this->turnOff();
}

void RGB::turnOnRed(){
	*(this->port) &= ~(1 << this->red);
	*(this->port) |= 1 << this->green;
	*(this->port) |= 1 << this->blue;
}

void RGB::turnOnBlue(){
	*(this->port) &= ~(1 << this->blue);
	*(this->port) |= 1 << this->green;
	*(this->port) |= 1 << this->red;
}

void RGB::turnOnGreen(){
	*(this->port) &= ~(1 << this->green);
	*(this->port) |= 1 << this->red;
	*(this->port) |= 1 << this->blue;
}

void RGB::turnOnYellow(){
	*(this->port) &= ~(1 << this->red);
	*(this->port) &= ~(1 << this->green);
	*(this->port) |= 1 << this->blue;
}

void RGB::turnOnWhite(){
	*(this->port) &= ~(1 << this->red);
	*(this->port) &= ~(1 << this->green);
	*(this->port) &= ~(1 << this->blue);
}

void RGB::turnOff(){
	*(this->port) |= 1 << this->green;
	*(this->port) |= 1 << this->red;
	*(this->port) |= 1 << this->blue;
}