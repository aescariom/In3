#ifndef _O_LED_H_
#define _O_LED_H_

#include <inttypes.h> //uint8_t
#include <util/delay.h> // delay

#define _O_LED_DELAY_MS 50

class Led
{
   	public:
    	Led(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask);
    	void init();
    	void blink();
    	void toggle();
    	void turnOn();
    	void turnOff();
      
	private:
		volatile uint8_t *port;
		volatile uint8_t *dir;
		uint8_t mask;
};

#endif