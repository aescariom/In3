#include "buzzer.h"

Buzzer::Buzzer(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask){
	this->port = port;
	this->dir = dir;
	this->mask = mask;
}

void Buzzer::init(){
	*(this->dir) |= 1 << this->mask; // output pin
}

void Buzzer::beep(){
	this->toggle();
    _delay_ms(_O_BUZZER_DELAY_MS);
	this->toggle();
}

void Buzzer::toggle(){
    *(this->port) ^= (1 << this->mask);
}
