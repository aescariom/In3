#ifndef _O_RGB_H_
#define _O_RGB_H_

#include <inttypes.h> //uint8_t

class RGB
{
   	public:
    	RGB(volatile uint8_t *port, volatile uint8_t *dir, volatile uint8_t *pin, uint8_t red, 
    		uint8_t green, uint8_t blue);
    	void init();
    	void turnOnRed();
    	void turnOnBlue();
    	void turnOnGreen();
    	void turnOnYellow();
    	void turnOnWhite();
    	void turnOff();
      
	private:
		volatile uint8_t *port;
		volatile uint8_t *dir;
		volatile uint8_t *pin;
		uint8_t red, green, blue;
};
#endif