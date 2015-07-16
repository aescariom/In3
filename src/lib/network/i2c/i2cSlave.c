#include "i2cSlave.h"

#include <stdio.h>

unsigned char regaddr, regdata;
volatile unsigned char waitingFor = 0;

void i2c_init_slave(){
    TWAR = my_addr;
    TWDR = 0x00;
    // SCL freq = F_CPU/(16+2(TWBR)*prescalerValue)
    TWBR = 32;  // Bit rate
    TWSR = (0 << TWPS1) | (0 << TWPS0); // Setting prescalar bits
    TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN) | (1<<TWIE);    // get ACK, en TWI, clear int flag
    sei();
}

void setValue(unsigned char data, unsigned char what){
	char line[16];
	if(what == 'T'){
		temperature = data;
	}else if(what == 'H'){
		humidity = data;
	}else if(what == 'S'){
		targetTemperature = data;
	}
}

void processData(unsigned char data){
	switch(data){
		case 'T':
		case 'H':
		case 'S':
			waitingFor = data;
			break;
		default:
			setValue(data, waitingFor);
			waitingFor = 0;
	}
}

ISR(TWI_vect){
  	static unsigned char last_val = 1;
  	// Disable Global Interrupt
  	cli();
  	switch(TW_STATUS){
    	case TW_START:  // 0x08
    	case TW_REP_START: // 0x10
    	case TW_MT_SLA_ACK: // 0x18
    	case TW_MT_SLA_NACK: // 0x20
    	case TW_MT_DATA_ACK: // 0x28
    	case TW_MT_DATA_NACK: // 0x30
    	case TW_MR_SLA_ACK:   // 0x40
    	case TW_MR_SLA_NACK:   // 0x48
    	case TW_MR_DATA_ACK:  // 0x50
    	case TW_MR_DATA_NACK:  //  0x58
    	case TW_ST_ARB_LOST_SLA_ACK:  //  0xB0
    	case TW_SR_ARB_LOST_SLA_ACK: //  0x68
    	case TW_SR_GCALL_ACK:    //0x70
    	case TW_SR_ARB_LOST_GCALL_ACK:  //  0x78
    	case TW_SR_GCALL_DATA_ACK:   // 0x90
    	case TW_SR_GCALL_DATA_NACK:  //  0x98
    	case TW_NO_INFO:   // 0xF8
      		TWCR |= (1<<TWINT);    // Clear TWINT Flag
      		break;
    	case TW_SR_SLA_ACK:      // 0x60: SLA+W received, ACK returned
      		TWCR |= (1<<TWINT);    // Clear TWINT Flag
      		break;
    	case TW_SR_DATA_ACK:     // data received
      		processData(TWDR);
      		TWCR |= (1<<TWINT);    // Clear TWINT Flag
      		break;
    	case TW_SR_STOP:         // 0xA0: stop or repeated start condition received while selected
      		TWCR |= (1<<TWINT);    // Clear TWINT Flag
      		break;
    	case TW_ST_SLA_ACK:      // 0xA8: SLA+R received, ACK returned
    		if(changeTargetTemp){
      			TWDR = targetTemperature;           // Fill TWDR register whith the data to be sent 
      		}else{
      			TWDR = 0;
      		}
      		changeTargetTemp = false;
      		TWCR = ((1 << TWEA) | (1 << TWINT) | (1 << TWEN) | (1 << TWIE));   // Enable TWI, Clear TWI interrupt flag
      		break;
    	case TW_ST_DATA_ACK:     // 0xB8: data transmitted, ACK received
      		TWCR |= (1<<TWINT);    // Clear TWINT Flag
      		break;
    	case TW_ST_DATA_NACK:    // 0xC0: data transmitted, NACK received
      		TWCR |= (1<<TWINT);    // Clear TWINT Flag
      		break;
    	case TW_ST_LAST_DATA:    // 0xC8: last data byte transmitted, ACK received
    	case TW_BUS_ERROR:       // 0x00: illegal start or stop condition
    	default:
      		TWCR |= (1<<TWINT);    // Clear TWINT Flag
      		break;
  	}

  	sei();
}