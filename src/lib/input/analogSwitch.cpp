#include "analogSwitch.h"
#include <avr/io.h>

AnalogSwitch::AnalogSwitch(uint8_t pin, uint16_t threshold){
	this->pin = pin;
	this->threshold = threshold;
}

void AnalogSwitch::init(){
}

bool AnalogSwitch::isPressed(){
    ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS0);
    ADMUX=this->pin;
    
    // starts the conversion
    ADCSRA |= (1<<ADSC);
    
    while (ADCSRA & (1<<ADSC));
    
    return ADCW < this->threshold;
}
