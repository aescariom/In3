#ifndef _I_ROTARY_H_
#define _I_ROTARY_H_

#include <inttypes.h>
#include <avr/io.h> // AVR ctes
#include <avr/interrupt.h> // interruptions

extern volatile int lastEncoded, encoderValue;
/*volatile int lastEncoded = 0;
volatile long encoderValue = 0;
volatile long lastEncoderValue = 0;*/

void updateEncoder();
void enableInt0();
void enableInt1();
void initRotary();

#endif