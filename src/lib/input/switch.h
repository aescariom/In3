#ifndef _I_SWITCH_H_
#define _I_SWITCH_H_

#include <inttypes.h>

class Switch
{
   	public:
    	Switch(volatile uint8_t *port, volatile uint8_t *dir, volatile uint8_t *pin, uint8_t mask);
    	void init();
    	bool isPressed();
      
	private:
		volatile uint8_t *port;
		volatile uint8_t *pin;
		volatile uint8_t *dir;
		uint8_t mask;
};

#endif