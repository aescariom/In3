#ifndef _I_ANALOG_SWITCH_H_
#define _I_ANALOG_SWITCH_H_

#include <inttypes.h>

class AnalogSwitch
{
   	public:
    	AnalogSwitch(uint8_t pin, uint16_t threshold);
    	void init();
    	bool isPressed();
      
	private:
        uint8_t pin;
        uint16_t threshold;
};

#endif
