PRG            = assy_spi
OBJ            = $(PRG).o  sw_spi.o
MCU_TARGET     = atmega128
OPTIMIZE       = -Os    # options are 1, 2, 3, s
DEFS           =
LIBS           =
CC             = avr-gcc
#override 
CFLAGS        = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)
#override 
LDFLAGS       = -Wl, -Map, $(PRG).map
OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump



all: $(PRG).elf lst text eeprom
	$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)


clean: 
	rm -rf *.o $(PRG).elf *.eps *.png *.pdf *.bak 
	rm -rf *.lst *.map $(EXTRA_CLEAN_FILES) *~

program: $(PRG).hex
	#sudo avrdude -p m128 -c usbasp -e -U flash:w:$(PRG).hex
	sudo avrdude -p atmega2560 -c usbtiny -e -U flash:w:$(PRG).hex
	lst:  $(PRG).lst
	%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@