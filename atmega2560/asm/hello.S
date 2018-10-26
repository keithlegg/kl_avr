#define __SFR_OFFSET 0	// keep io.h from adding an incorrect offset to our port addresses
#include <avr/io.h>
.global main
main:
    ldi r16,0xff
    sts DDRK,r16
    rcall Start

Start:
    ldi r16,0xff
    sts PORTK,r16
    
    ldi r16,0x00
    sts PORTK,r16
    rjmp Start
    