#ifndef _O_BUZZER_H_
#define _O_BUZZER_H_

#include <inttypes.h> //uint8_t
#include <util/delay.h> // delay
#define _O_BUZZER_DELAY_MS 50

class Buzzer
{
   	public:
    	Buzzer(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask);
    	void init();
    	void beep();
    	void toggle();
      
	private:
		volatile uint8_t *port;
		volatile uint8_t *dir;
		uint8_t mask;
};

#endif
