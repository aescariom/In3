#ifndef _O_RELAY_H_
#define _O_RELAY_H_

#include <inttypes.h> //uint8_t

class Relay
{
   	public:
    	Relay(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask);
    	void init();
    	void open();
    	void close();
      
	private:
		volatile uint8_t *port;
		volatile uint8_t *dir;
		uint8_t mask;
};

#endif