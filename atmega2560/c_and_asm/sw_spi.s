//sw_spi.S, R. Traylor, 12.1.08

#include <avr/io.h>
.text
.global sw_spi

//define the pins and ports, using PB0,1,2
.equ  spi_port , 0x18  ;PORTB 
.equ  mosi     ,    0  ;PB2 pin
.equ  sck      ,    1  ;PB0 pin
.equ  cs_n     ,    2  ;PB1 pin

//r18 counts to eight, r24 holds data byte passed in
sw_spi: ldi r18,0x08         ;setup counter for eight clock pulses
        cbi spi_port, cs_n   ;set chip select low
loop:   rol r24              ;shift byte left (MSB first); carry set if bit7 is one
        brcc bit_low         ;if carry not true, bit was zero, not one
        sbi spi_port, mosi   ;set port data bit to one
		rjmp clock           ;ready for clock pulse
bit_low: cbi spi_port, mosi  ;set port data bit to zero  
clock:  sbi spi_port, sck    ;sck -> one  
		cbi spi_port, sck    ;sck -> zero
		dec r18              ;decrement the bit counter
		brne loop            ;loop if not done
		sbi spi_port, cs_n   ;dessert chip select to high 
		ret                  ;
.end
