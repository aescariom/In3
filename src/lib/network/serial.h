#ifndef _H_SERIAL_
#define _H_SERIAL_ 1

#ifndef F_CPU
	#define F_CPU 8000000
#endif
#ifndef BAUD
    #define BAUD 9600
#endif

#define R_UBRR F_CPU/16/BAUD-1

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

void usart_init(unsigned int);
void usart_putchar(unsigned char data); 
void usart_putstring(char data[]); 
int usart_putchar_printf(char, FILE *);

#endif // _H_SERIAL_ 