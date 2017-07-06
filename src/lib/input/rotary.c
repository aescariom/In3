#include "rotary.h"
#include <util/delay.h>

volatile int encoderValue = 0;
volatile int lastEncoded = 0;
volatile int invocations = 0;

void updateEncoder(){
	cli();
	int MSB = (PIND & ( 1 << PD2 )) ? 1 : 0; //MSB = most significant bit
	int LSB = (PIND & ( 1 << PD3 )) ? 1 : 0; //LSB = least significant bit

	int encoded = (MSB << 1) | LSB; //converting the 2 pin value to single number

	int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value

	if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue --;
	if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue ++; // Ver lo que está fallando

	lastEncoded = encoded; //store this value for next time
  	sei();
}

void enableInt0(){
	DDRD &= ~(1 << DDD2);     // Clear the PD2 pin
    // PD2 (PCINT0 pin) is now an input

    PORTD |= (1 << PORTD2);    // turn On the Pull-up
    // PD2 is now an input with pull-up enabled
    EICRA |= (1 << ISC00);    // set INT0 to trigger on ANY logic change
    EIMSK |= (1 << INT0);     // Turns on INT0
}

void enableInt1(){
	 DDRD &= ~(1 << DDD3);     // Clear the PD3 pin
    // PD3 (PCINT1 pin) is now an input

    PORTD |= (1 << PORTD3);    // turn On the Pull-up
    // PD3 is now an input with pull-up enabled
    EICRA |= (1 << ISC10);    // set INT1 to trigger on ANY logic change
    EIMSK |= (1 << INT1);     // Turns on INT1
}

void initRotary(){
	enableInt0();
	enableInt1();
	sei();
}

ISR (INT0_vect){ // PD2
    updateEncoder();
}

ISR (INT1_vect){ // PD3 is never called
    updateEncoder();
}