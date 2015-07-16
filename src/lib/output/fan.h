#ifndef _O_FAN_H_
#define _O_FAN_H_

#include <inttypes.h> //uint8_t

class Fan
{
   	public:
    	Fan(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask);
    	void init();
    	void turnOn();
    	void turnOff();
      
	private:
		volatile uint8_t *port;
		volatile uint8_t *dir;
		uint8_t mask;
};

#endif