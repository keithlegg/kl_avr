#define __SFR_OFFSET 0	// keep io.h from adding an incorrect offset to our port addresses
#include <avr/io.h>
.global main
main:
    ldi r16,0xff
    out DDRB,r16
    rcall Start

Start:
    ldi r16,0xff
    out PORTB,r16
    
    ldi r16,0x00
    out PORTB,r16
    rjmp Start
    