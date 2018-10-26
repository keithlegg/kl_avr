
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

   rjmp    preflash //showramend


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
    ldi r16, 5
longerdelay:
    rcall prelongdelay
    dec r16
    brne longerdelay
    pop r16
    ret 

//-----------------------
preflash:
   ldi r18, 10
flash_signal:
   //// 
   ldi r17, 0xff
   out PORTE, r17
   rcall prelongerdelay
   //// 
   ldi r17, 0
   out PORTE, r17
   rcall prelongerdelay

   dec r18
   brne flash_signal
   //ret  

//-----------------------
//blink the high and low bits on PORTE for RAMEND with a pause between

showramend:

   ldi r17, HI(RAMEND)
   out PORTE, r17   
   rcall prelongerdelay
   //// 
   ldi r17, 0
   out PORTE, r17
   rcall prelongerdelay
   ////
   ldi r17, LO(RAMEND)
   out PORTE, r17 
   rcall prelongerdelay
   ////
   ldi r17, 0
   out PORTE, r17
   rcall prelongerdelay
   ////   
   rcall prelongerdelay
   rcall prelongerdelay


   rjmp showramend


//-----------------------

