#ifndef _H_I2CSLAVE_
#define _H_I2CSLAVE_ 1

#include <avr/interrupt.h>
#include <util/twi.h>

extern volatile unsigned char temperature; // TODO
extern volatile unsigned char humidity;    // TODO
extern volatile unsigned char targetTemperature;    // TODO
extern volatile bool changeTargetTemp;    // TODO
extern unsigned char my_addr;

void i2c_init_slave();
void setValue(unsigned char data, unsigned char what);
void processData(unsigned char data);

#endif // _H_SERIAL_ 