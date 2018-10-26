

## The "C" way 
## avr-gcc -DF_CPU=16000000UL -mmcu=atmega164p -o blink.out blink.c 
## avr-objcopy -O ihex -R .eeprom blink.out blink.hex
## avrdude -V -c usbtiny -p atmega164p -b 19200 -P /dev/cuaU0 -U flash:w:blink.hex

###############################

# compile C to assembly
#avr-g++ -S -DF_CPU=16000000UL -mmcu=attiny85 -o blink.S src/blink.c

###############################


# create an object file
avr-g++ -DF_CPU=16000000UL -mmcu=attiny85 -c blink.S -o blink.o

# invoke the linker ??
#gcc blink.o -o blink
avr-objcopy -O ihex -R .eeprom blink.o blink.hex 

# program the device - NEEDS SUDO - GRR
avrdude -V -c usbtiny -p attiny85 -b 19200 -U flash:w:blink.hex


