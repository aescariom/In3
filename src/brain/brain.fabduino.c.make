PROJECT=brain.fabduino
SOURCES=../lib/network/serial.c ../lib/output/rgb.cpp  ../lib/output/lcd.cpp ../lib/input/switch.cpp ../lib/input/rotary.c ../lib/operators/new.c $(PROJECT).c
MMCU=atmega168
F_CPU = 16000000

CFLAGS=-mmcu=$(MMCU) -Wall -Os -DF_CPU=$(F_CPU)

$(PROJECT).hex: $(PROJECT).out
	avr-objcopy -O ihex $(PROJECT).out $(PROJECT).c.hex;\
	avr-size --mcu=$(MMCU) --format=avr $(PROJECT).out
 
$(PROJECT).out: $(SOURCES)
	avr-c++ $(CFLAGS) -I./ -o $(PROJECT).out $(SOURCES)
 
program-bsd: $(PROJECT).hex
	avrdude -p $(MMCU) -c bsd -U flash:w:$(PROJECT).c.hex

program-dasa: $(PROJECT).hex
	avrdude -p $(MMCU) -P /dev/ttyUSB0 -c dasa -U flash:w:$(PROJECT).c.hex

program-avrisp2: $(PROJECT).hex
	avrdude -p $(MMCU) -P usb -c avrisp2 -U flash:w:$(PROJECT).c.hex

program-avrisp2-fuses: $(PROJECT).hex
	avrdude -p $(MMCU) -P usb -c avrisp2 -U lfuse:w:0x56:m
	avrdude -p $(MMCU) -P usb -c avrisp2 -U hfuse:w:0xD9:m
	avrdude -p $(MMCU) -P usb -c avrisp2 -U efuse:w:0x07:m

program-usbtiny: $(PROJECT).hex
	avrdude -p $(MMCU) -P usb -c usbtiny -U flash:w:$(PROJECT).c.hex

program-usbtiny-fuses: $(PROJECT).hex
	avrdude -p $(MMCU) -P usb -c usbtiny -U lfuse:w:0xD6:m
	avrdude -p $(MMCU) -P usb -c usbtiny -U hfuse:w:0xD9:m
	avrdude -p $(MMCU) -P usb -c usbtiny -U efuse:w:0x07:m

program-dragon: $(PROJECT).hex
	avrdude -p $(MMCU) -P usb -c dragon_isp -U flash:w:$(PROJECT).c.hex
