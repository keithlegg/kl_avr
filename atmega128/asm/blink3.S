
#define __SFR_OFFSET 0     // Needed to keep io.h from adding an incorrect offset to our port addresses
#include <avr/io.h>


#define LO(x) ((x) & 0xFF) 
#define HI(x) ((x)>>8) 

#define accum r16

.global main               // Main entry point


//-----------------------

main:
   // Configure Port B for control signals
   ldi      accum, 0xff     
 
   //configure IO registers 
   sts      DDRF,  accum  //#PORTF WANTS STS -I/O PORT? 
   out      DDRA,  accum   
   out      DDRE,  accum
   out      DDRC,  accum   

   // Configure Port C for video signals
   ldi      accum, 0xff  
   out      DDRC,  accum

   // Put our stack at the top of internal SRAM
   ldi      accum, HI(RAMEND)
   out      SPH,   accum
   ldi      accum, LO(RAMEND)
   out      SPL,   accum

   rjmp    foo


//-----------------------
predelay:
    push r16
    ldi r16, 255
delay:
    dec r16
    brne delay
    pop r16
    ret 

//-----------------------
prelongdelay:
    push r16
    ldi r16, 255
longdelay:
    rcall predelay
    dec r16
    brne longdelay
    pop r16
    ret 

//-----------------------
prelongerdelay:
    push r16
    ldi r16, 2
longerdelay:
    rcall prelongdelay
    dec r16
    brne longerdelay
    pop r16
    ret 

//-----------------------


//-----------------------

foo:
    ldi r17,0

bar:
   //mov accum, r17
   sts PORTF, r17 
   out PORTA, r17 
   
   out PORTE, r17 
   out PORTC, r17 

   rcall prelongerdelay

   ldi accum, 0x00
   sts PORTF, accum 
   rcall prelongerdelay

   inc r17
   brne bar
   rjmp foo


//-----------------------

