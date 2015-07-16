#ifndef _O_LCD_H_
#define _O_LCD_H_

#include <inttypes.h> //uint8_t
#include <util/delay.h> // delay

class LCD
{
   	public:
    	LCD();
    	void init();
    	void cmd(char byte);
    	void setDb7(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask);
    	void setDb6(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask);
    	void setDb5(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask);
    	void setDb4(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask);
    	void setE(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask);
    	void setRs(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask);
    	void putChar(char byte);
    	void erase();
		void cursorOff();
		void putString(char* message);
		void println(char* message, int line);
      
	protected:
		void delay();
		void strobeDelay();
      
	private:
		volatile uint8_t *port_db7, *port_db6, *port_db5, *port_db4, *port_e, *port_rs;
		volatile uint8_t *dir_db7, *dir_db6, *dir_db5, *dir_db4, *dir_e, *dir_rs;
		uint8_t mask_db7, mask_db6, mask_db5, mask_db4, mask_e, mask_rs;
};
#endif