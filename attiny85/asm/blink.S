
#define __SFR_OFFSET 0		// Needed to keep io.h from adding an incorrect offset to our port addresses
#include <avr/io.h>


#define LO(x) ((x) & 0xFF) 
#define HI(x) ((x)>>8) 

#define hsync 0
#define vsync 1
#define blockWidth 60
#define blocksPerRow 8

#define accum r16
#define rowColorStart r19
#define blockCount r20
#define pixelCount r22
#define rgb r23					// xxRRGGBB



/*
   reset|PB5    .--|_|--.      VCC
   PB3          |       |  PB2|SCK
   PB4          |       |  PB1|MISO
   GND          |_______|  PB0|MOSI
*/


.global main					// Main entry point
//.global TIMER1_COMPA_vect		// Video interrupt service routine


main:
	// Configure Port B for control signals
	ldi		accum,0xff		 
	out		DDRB,accum

	// Put our stack at the top of internal SRAM
	ldi		accum, LO(RAMEND)
	out		SPL, accum
	ldi		accum, HI(RAMEND)
	out		SPH, accum

	ldi		accum, _BV(CLKPCE)
	sts		CLKPR, accum
	ldi		accum, 0
	sts		CLKPR, accum
   rjmp    bar

foo:
	nop

bar:
    .equ blah, 0xff
    ldi r17, blah
    out PORTB,r17
    
    ldi r17,0
    out PORTB, r17

	rjmp    foo





