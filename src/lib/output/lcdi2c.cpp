#include "lcdi2c.h"

/* PUBLIC */

LCD_I2C::LCD_I2C(uint8_t addr, uint8_t rows, uint8_t cols){
	this->addr = addr;
	this->rows = rows;
	this->cols = cols;
}

void LCD_I2C::init(){
    this->displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
    begin(this->cols, this->rows);
}

void LCD_I2C::send(uint8_t value, uint8_t mode) {
	uint8_t highnib=value&0xf0;
	uint8_t lownib=(value<<4)&0xf0;
    write4bits((highnib)|mode);
	write4bits((lownib)|mode); 
}

void LCD_I2C::command(uint8_t value) {
	this->send(value, 0);
}

void LCD_I2C::clear(){
	this->command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	_delay_us(2000);  // this command takes a long time!
}

void LCD_I2C::home(){
	this->command(LCD_RETURNHOME);  // set cursor position to zero
	_delay_us(2000);  // this command takes a long time!
}

void LCD_I2C::display() {
	this->displaycontrol |= LCD_DISPLAYON;
	this->command(LCD_DISPLAYCONTROL | this->displaycontrol);
}

void LCD_I2C::setCursor(uint8_t row, uint8_t col){
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if ( row > this->numlines ) {
		row = this->numlines-1;    // we count rows starting w/0
	}
	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void LCD_I2C::backlight(int val){
	if(val == 1){
		this->backlightval=LCD_BACKLIGHT;
	}else{
		this->backlightval=LCD_NOBACKLIGHT;
	}
	expanderWrite(0);
}

void LCD_I2C::display(int val){
	if(val != 1){
		this->displaycontrol &= ~LCD_DISPLAYON;
	}else{
		this->displaycontrol |= LCD_DISPLAYON;
	}
	expanderWrite(0);
}

void LCD_I2C::cursor(int val){
	if(val != 1){
		this->displaycontrol &= ~LCD_CURSORON;
	}else{
		this->displaycontrol |= LCD_CURSORON;
	}
	expanderWrite(0);
}

void LCD_I2C::blink(int val){
	if(val != 1){
		this->displaycontrol &= ~LCD_BLINKON;
	}else{
		this->displaycontrol |= LCD_BLINKON;
	}
	expanderWrite(0);
}

void LCD_I2C::println(char* str, uint8_t line){
	uint8_t col = 0;
	this->setCursor(line, col);
	this->print(str);
}

void LCD_I2C::print(char* str){
	static uint8_t i;
   	static char chr;
   	i = 0;
   	while (1) {
   	   chr = str[i];
   	   if (chr == 0)
   	      return;
   	   this->send(chr, Rs);
   	   ++i;
   	}
}


/* PROTECTED */
void LCD_I2C::expanderWrite(uint8_t _data){                                        
	i2c_start_wait((this->addr<<1) + I2C_WRITE);
	i2c_write((int)(_data) | this->backlightval);
	i2c_stop();   
}

void LCD_I2C::pulseEnable(uint8_t _data){
	expanderWrite(_data | En);	// En high
	_delay_us(1);		// enable pulse must be >450ns
	
	expanderWrite(_data & ~En);	// En low
	_delay_us(50);		// commands need > 37us to settle
} 

void LCD_I2C::write4bits(uint8_t value) {
	expanderWrite(value);
	pulseEnable(value);
}

void LCD_I2C::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
	if (lines > 1) {
		this->displayfunction |= LCD_2LINE;
	}
	this->numlines = lines;

	// for some 1 line displays you can select a 10 pixel high font
	if ((dotsize != 0) && (lines == 1)) {
		this->displayfunction |= LCD_5x10DOTS;
	}

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	_delay_ms(50); 
  
	// Now we pull both RS and R/W low to begin commands
	expanderWrite(this->backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
	_delay_ms(1000);

  	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46
	
	  // we start in 8bit mode, try to set 4 bit mode
   write4bits(0x03 << 4);
   _delay_us(4500); // wait min 4.1ms
   
   // second try
   write4bits(0x03 << 4);
   _delay_us(4500); // wait min 4.1ms
   
   // third go!
   write4bits(0x03 << 4); 
   _delay_us(150);
   
   // finally, set to 4-bit interface
   write4bits(0x02 << 4); 


	// set # lines, font size, etc.
	command(LCD_FUNCTIONSET | this->displayfunction);  
	
	// turn the display on with no cursor or blinking default
	this->displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	display();
	
	// clear it off
	clear();
	
	// Initialize to default text direction (for roman languages)
	this->displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	
	// set the entry mode
	command(LCD_ENTRYMODESET | this->displaymode);
	
	home();  
}