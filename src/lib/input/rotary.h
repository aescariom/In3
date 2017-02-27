#ifndef _I_ROTARY_H_
#define _I_ROTARY_H_

#include <inttypes.h>
#include <avr/io.h> // AVR ctes
#include <avr/interrupt.h> // interruptions

extern volatile int encoderValue;

void updateEncoder();
void enableInt0();
void enableInt1();
void initRotary();

#endif