
PROJECT=blink


###########################
#YOU CAN INCLUDE PATHS TO OTHER C/CPP FILES HERE 

#SOURCES=src/other_awesome_c_file.c

SOURCES=src/blink.c


###########################


CC=avr-g++


OBJCOPY=avr-objcopy
MMCU=atmega128
PROG_MMCU=atmega128
PROGRAMMER=usbtiny
PORT=usb

#defaults for mega128
LOW_FUSE=0xc1
HIGH_FUSE=0x99
EXT_FUSE=0xfd


CFLAGS=-mmcu=$(MMCU) -Wall -Os -std=gnu99 -funsigned-char \
	-funsigned-bitfields -fpack-struct -fshort-enums -MD -MP -MT -I./

$(PROJECT).hex: $(PROJECT).out
	@echo "Creating hex file..."
	$(OBJCOPY) -j .text -O ihex $(PROJECT).out $(PROJECT).hex
	rm -f ./*.d
	rm -f ./*.out
	@echo

$(PROJECT).out: $(SOURCES)
	@echo
	@echo "Compiling..."
	$(CC) $(CFLAGS) -o $(PROJECT).out $(SOURCES)
	avr-size $(PROJECT).out
	@echo

program: $(PROJECT).hex
	@echo
	@echo "Downloading..."
	avrdude -P $(PORT) -v -p $(PROG_MMCU) -c $(PROGRAMMER) -e \
	-U flash:w:$(PROJECT).hex
	@echo

program_fuses:
	@echo
	@echo "Writing fuses..."
	avrdude -P $(PORT) -v -p $(PROG_MMCU) -c $(PROGRAMMER) -e \
	-U lfuse:w:$(LOW_FUSE):m -U hfuse:w:$(HIGH_FUSE):m -U efuse:w:$(EXT_FUSE):m
	@echo

clean:
	@echo
	@echo "Cleaning..."
	rm -f ./*.out
	rm -f ./*.hex
	rm -f ./*.d
	rm -f ./*.map
	rm -f ./*.o
	@echo

