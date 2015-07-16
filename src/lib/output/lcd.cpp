#include "lcd.h"

LCD::LCD(){
}

void LCD::setDb7(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask){
	this->port_db7 = port;
	this->dir_db7 = dir;
	this->mask_db7 = mask;
}

void LCD::setDb6(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask){
	this->port_db6 = port;
	this->dir_db6 = dir;
	this->mask_db6 = mask;
}

void LCD::setDb5(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask){
	this->port_db5 = port;
	this->dir_db5 = dir;
	this->mask_db5 = mask;
}

void LCD::setDb4(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask){
	this->port_db4 = port;
	this->dir_db4 = dir;
	this->mask_db4 = mask;
}

void LCD::setE(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask){
	this->port_e = port;
	this->dir_e = dir;
	this->mask_e = mask;
}

void LCD::setRs(volatile uint8_t *port, volatile uint8_t *dir, uint8_t mask){
	this->port_rs = port;
	this->dir_rs = dir;
	this->mask_rs = mask;
}

void LCD::init(){
	// pin initialization
	*(this->port_db7) &= ~(1 << this->mask_db7);
	*(this->dir_db7) |= 1 << this->mask_db7; // output pin
	*(this->port_db6) &= ~(1 << this->mask_db6);
	*(this->dir_db6) |= 1 << this->mask_db6; // output pin
	*(this->port_db5) &= ~(1 << this->mask_db5);
	*(this->dir_db5) |= 1 << this->mask_db5; // output pin
	*(this->port_db4) &= ~(1 << this->mask_db4);
	*(this->dir_db4) |= 1 << this->mask_db4; // output pin
	*(this->port_e) &= ~(1 << this->mask_e);
	*(this->dir_e) |= 1 << this->mask_e; // output pin
	*(this->port_rs) &= ~(1 << this->mask_rs);
	*(this->dir_rs) |= 1 << this->mask_rs; // output pin
	
	// wait until it is powered on
	this->delay();
	
   	// initialization sequence
    this->cmd(0b001100); // (1 << this->mask_db5) + (1 << this->mask_db4)
    this->cmd(0b001100); // (1 << this->mask_db5) + (1 << this->mask_db4)
    this->cmd(0b001100); // (1 << this->mask_db5) + (1 << this->mask_db4)
   
   	// configuring 4 pin interface
	this->cmd(0b000100); // (1 << this->mask_db5)
   
   	// two lines LCD, 5x7
	this->cmd(0b000100);//(1 << this->mask_db5)
	this->cmd(0b000001); // (1 << this->mask_db7)
	
	// turn on display
	this->cmd(0);
    this->cmd(0b000111); // (1 << this->mask_db7) + (1 << this->mask_db6) + (1 << this->mask_db5)
    
    //setting entry mode
	this->cmd(0);
    this->cmd(0b0000110); //(1 << this->mask_db6) + (1 << this->mask_db5)
}

void LCD::cmd(char byte){
	// clear RS
	*(this->port_rs) &= ~(1 << this->mask_rs);
	
   //
   if(byte & (1 << 5)){
		*(this->port_rs) |= 1 << this->mask_rs;
   }else{
		*(this->port_rs) &= ~(1 << this->mask_rs);
   }
   if(byte & (1 << 4)){
		*(this->port_e) |= 1 << this->mask_e;
   }else{
		*(this->port_e) &= ~(1 << this->mask_e);
   }
   if(byte & (1 << 3)){
		*(this->port_db4) |= 1 << this->mask_db4;
   }else{
		*(this->port_db4) &= ~(1 << this->mask_db4);
   }
   if(byte & (1 << 2)){
		*(this->port_db5) |= 1 << this->mask_db5;
   }else{
		*(this->port_db5) &= ~(1 << this->mask_db5);
   }
   if(byte & (1 << 1)){
		*(this->port_db6) |= 1 << this->mask_db6;
   }else{
		*(this->port_db6) &= ~(1 << this->mask_db6);
   }
   if(byte & (1 << 0)){
		*(this->port_db7) |= 1 << this->mask_db7;
   }else{
		*(this->port_db7) &= ~(1 << this->mask_db7);
   }
   
   // strobe E
   this->strobeDelay();
   *(this->port_e) |= 1 << this->mask_e;
   this->strobeDelay();
   *(this->port_e) &= ~(1 << this->mask_e);
   
   // wait
   this->delay();
}

void LCD::putChar(char byte){

   // set RS to send data
   *(this->port_rs) |= 1 << this->mask_rs;
   
   //
   // output high nibble
   //
   if(byte & (1 << 7)){
	*(this->port_db7) |= 1 << this->mask_db7; // output pin
   }else{
	*(this->port_db7) &= ~(1 << this->mask_db7); // output pin
   }
   if(byte & (1 << 6)){
	*(this->port_db6) |= 1 << this->mask_db6; // output pin
   }else{
	*(this->port_db6) &= ~(1 << this->mask_db6); // output pin
   }
   if(byte & (1 << 5)){
	*(this->port_db5) |= 1 << this->mask_db5; // output pin
   }else{
	*(this->port_db5) &= ~(1 << this->mask_db5); // output pin
   }
   if(byte & (1 << 4)){
	*(this->port_db4) |= 1 << this->mask_db4; // output pin
   }else{
	*(this->port_db4) &= ~(1 << this->mask_db4); // output pin
   }
   
   //
   // strobe E
   //
   this->strobeDelay();
   *(this->port_e) |= 1 << this->mask_e;
   this->strobeDelay();
   *(this->port_e) &= ~(1 << this->mask_e);
   //
   // wait
   //
   this->delay();
   //
   // output low nibble
   //
   if(byte & (1 << 3)){
	*(this->port_db7) |= 1 << this->mask_db7; // output pin
   }else{
	*(this->port_db7) &= ~(1 << this->mask_db7); // output pin
   }
   if(byte & (1 << 2)){
	*(this->port_db6) |= 1 << this->mask_db6; // output pin
   }else{
	*(this->port_db6) &= ~(1 << this->mask_db6); // output pin
   }
   if(byte & (1 << 1)){
	*(this->port_db5) |= 1 << this->mask_db5; // output pin
   }else{
	*(this->port_db5) &= ~(1 << this->mask_db5); // output pin
   }
   if(byte & (1 << 0)){
	*(this->port_db4) |= 1 << this->mask_db4; // output pin
   }else{
	*(this->port_db4) &= ~(1 << this->mask_db4); // output pin
   }
   //
   // strobe E
   //
   this->strobeDelay();
   *(this->port_e) |= 1 << this->mask_e;
   this->strobeDelay();
   *(this->port_e) &= ~(1 << this->mask_e);
   //
   // wait and return
   //
   this->delay();
}

void LCD::delay(){
	_delay_ms(10);
}

void LCD::strobeDelay(){
	_delay_us(1);
}

void LCD::erase(){
   this->cmd(0);
   this->cmd(0b001000);   
}

void LCD::cursorOff(){
   this->cmd(0);
   this->cmd(0b000011);
}

void LCD::putString(char* message) {
   static uint8_t i;
   static char chr;
   i = 0;
   while (1) {
      chr = message[i];
      if (chr == 0)
         return;
      this->putChar(chr);
      ++i;
   }
}

void LCD::println(char* message, int line){
   if(line == 1){
      this->cmd(0);
      this->cmd(0b100);
   }else if(line == 2){
      this->cmd(0b11);
      this->cmd(0);
   }
   this->putString(message);
}