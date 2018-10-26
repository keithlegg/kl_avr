


SOURCES =  main.c kernel_splatr.c ST7735.c sram.c nes_ctrlr.c uart.c 


#avr-gcc main.c kernel_splatr.c


#You are compiling only one file not both. More precisely, you are not linking the files together.
#I don't know your compiler, but with gcc, it would be something like this:
#
#gcc -c main.c          <-- compile only
#gcc -c checksum.c      <-- compile only
#gcc main.o checksum.o  <-- link the two


###################################
PROJECT=blink
CC=avr-gcc
OBJCOPY=avr-objcopy
MMCU=atmega128

PROG_MMCU=atmega128
PROGRAMMER=usbtiny

#defaults for mega128
LOW_FUSE=0xc1
HIGH_FUSE=0x99
EXT_FUSE=0xfd

CFLAGS=-mmcu=$(MMCU) -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums 


# CC     = gcc
# DEBUG  = -ggdb3
# CFLAGS = $(DEBUG) -Wall
# LDLIBS = -lm -lpthread -lrt -l
# PROGS  = file1 file2 
# all: $(PROGS) 
# clean:
#     rm -f $(PROGS) *.o *~


$(PROJECT).hex: $(PROJECT).out
	@echo "Creating hex file..."
	#$(OBJCOPY) -j .text -O ihex $(PROJECT).out $(PROJECT).hex
	$(OBJCOPY) -j .text -j .data -O ihex $(PROJECT).out $(PROJECT).hex

	rm -f ./*.d
	rm -f ./*.out
	@echo

$(PROJECT).out: $(SOURCES)
	@echo
	@echo "Compiling..."
	$(CC) $(CFLAGS) -I./ -o $(PROJECT).out $(SOURCES) 
	avr-size $(PROJECT).out
	@echo

program_fuses:
	@echo
	@echo "Writing fuses..."
	avrdude -P $(PORT) -v -p $(PROG_MMCU) -c $(PROGRAMMER) -e \
	-U lfuse:w:$(LOW_FUSE):m -U hfuse:w:$(HIGH_FUSE):m -U efuse:w:$(EXT_FUSE):m
	@echo

program: $(PROJECT).hex
	@echo
	@echo "Downloading..."
	sudo avrdude -v -p $(PROG_MMCU) -c $(PROGRAMMER) -e \
	-U flash:w:$(PROJECT).hex
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

