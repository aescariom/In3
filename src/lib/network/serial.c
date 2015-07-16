#include "serial.h"

// http://www.atmel.com/webdoc/AVRLibcReferenceManual/group__avr__stdio_1gaea2b6be92ead4673bc487b271b7227fb.html
//static FILE serial_stdout = FDEV_SETUP_STREAM(usart_putchar_printf, NULL, _FDEV_SETUP_WRITE);

void usart_init(unsigned int ubrr) {
    // overwritting the stdout. From now on, printf will send data through the serial port
    //stdout = &serial_stdout;
    // baud rate
    UBRR0H = (unsigned char)(ubrr>>8); 
    UBRR0L = (unsigned char)ubrr;
	// enabling TX, RX & interrupt
    UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
    // word length = 8 bit + 1 stop bit
    UCSR0C = (1<<UCSZ00) | (1 << UCSZ01);
}

void usart_putchar(unsigned char data) {
    // wait until the buffer is empty
    while (!( UCSR0A & (1<<UDRE0)));
    UDR0 = data;
}

void usart_putstring(char arr[]) {
    int i = 0;
    int size = 16;//sizeof(arr)/sizeof(arr[0]);
    for(i = 0; i < size; ++i){
    	usart_putchar(arr[i]);
    }
}

int usart_putchar_printf(char var, FILE *stream) {
    if (var == '\n') {
        usart_putchar('\r');
    }
    usart_putchar(var);
    return 0;
}