PROJECT=blink

##########################
SOURCES=./src/analog_read_tiny13.c


###########################
CC=avr-gcc
OBJCOPY=avr-objcopy
MMCU=attiny13
PROG_MMCU=attiny13
PROGRAMMER=usbtiny
PORT=usb

#defaults for attiny13
LOW_FUSE=0x6a
HIGH_FUSE=0xff
EXT_FUSE=0xff

###############################

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
	@echo "Compiling..."SOURCES=./src/blink.c
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

