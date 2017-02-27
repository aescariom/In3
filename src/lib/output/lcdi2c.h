#ifndef _O_LCDI2C_H_
#define _O_LCDI2C_H_

#include <inttypes.h> //uint8_t
#include <util/delay.h> // delay
#include <util/twi.h>
#include "../lib/ext/i2chw/i2cmaster.h"

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define Rs 0b00000001  // Register select bit
#define Rw 0b00000010  // Read/Write bit
#define En 0b00000100  // Enable bit

class LCD_I2C{
   	public:
    	LCD_I2C(uint8_t, uint8_t, uint8_t);
    	void init();
    	void send(uint8_t, uint8_t);
    	void command(uint8_t);
    	void clear();
    	void home();
    	void display();
    	void setCursor(uint8_t, uint8_t);
    	void backlight(int);
    	void cursor(int);
    	void display(int);
    	void blink(int);
    	void print(char*);
    	void println(char*, uint8_t);
      
	protected:
		void expanderWrite(uint8_t);
		void pulseEnable(uint8_t);
		void write4bits(uint8_t);
		void begin(uint8_t, uint8_t, uint8_t charsize = LCD_5x8DOTS);
      
	private:
		uint8_t addr;
		uint8_t displayfunction;
		uint8_t displaycontrol;
		uint8_t displaymode;
		uint8_t numlines;
		uint8_t cols;
		uint8_t rows;
		uint8_t backlightval;
};
#endif