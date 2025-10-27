#include <avr/io.h>

#define __SFR_OFFSET 0	// keep io.h from adding an incorrect offset to our port addresses



.global main

;.equ iVal, 39998     ; inner loop value


//-----------------------

main:
    ;data direction out PORTD
    ldi r17, 0xff
    out DDRD,r17
    rcall start

start:  
    rjmp myloop



;predelay:
;    ;push r16
;    ldi r16, 255
;    ret
delay:
    dec r16
    brne delay
    ;pop r16
    ret 

 


myloop:  
    
    pulse_out

    rjmp   myloop    


pulse_out:

    ;initial long pulse 
    ldi r17, 0x04

    ;end on a low 
    ldi r17, 0x00
    out PORTD,r17  

    ret


