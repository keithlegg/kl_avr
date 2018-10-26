
#define __SFR_OFFSET 0     // Needed to keep io.h from adding an incorrect offset to our port addresses
#include <avr/io.h>


#define LO(x) ((x) & 0xFF) 
#define HI(x) ((x)>>8) 

#define accum r16

.global main               // Main entry point


.equ clrr, 0xf0

//-----------------------

main:
   // Configure Port B for control signals
   ldi      accum, 0xff     
   
   //out      DDRB,  accum
   sts      DDRF,  accum  //#PORTF WANTS STS -I/O PORT?


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
    ldi r16, 40
longerdelay:
    rcall prelongdelay
    dec r16
    brne longerdelay
    pop r16
    ret 

//-----------------------


//-----------------------

//load buufer into ram - not worky
// LDI XL, LOW(NRWW_START_ADDR)
// LDI XH, HIGH(NRWW_START_ADDR)
// loadBuffer:
// IN R0,PINB
// ST X+,R0
// CPI XH,0x4B
// BRNE loadBuffer


foo:
    //set 16 bit data in Z pointer
    LDI ZL, 0xaa
    LDI ZH, 0x04  //hex 1024 //dec 0x0400
    
    adiw r30, 1 

    mov accum, ZL 
    sts PORTF, accum      //display byte showing copy of XL byte 
    ret




//foo:
//    //load value into sram 
//    ldi accum, clrr
//    sts 0x0060, accum
//    //clear output port 
//    ldi accum, 0x00 
//    sts PORTF, accum
//    rcall prelongerdelay
//    //show sram in output port
//    lds accum, 0x0060
//    sts PORTF, accum
//    rcall prelongerdelay
//    ldi accum, 0x00 
//    rjmp foo


//-----------------------

//-----------------------







//-----------------------
//logical shift demo 
// foo:
//    ldi accum, 0xFF
//    ldi r17, 9
// bar:
//    sts PORTF, accum 
//    lsl accum
//    //ldi r18, 0xf0
//    //and accum, r18 
//    rcall prelongerdelay
//    sts PORTF, accum 
//    dec r17
//    brne bar
//    rjmp foo

//-----------------------
// //test bit in status register and conditional branch 
// foo:
//    ldi accum, 0xff
//    sts PORTF, accum 
//    rcall prelongerdelay   
//    ret
// baz:
//    ldi accum, 0x0f
//    sts PORTF, accum 
//    rcall prelongerdelay   
//    ret
// bar:
//    ldi accum, 0x00
//    sts PORTF, accum 
//    rcall prelongerdelay
//    //cli    //toglr
//    sei  //toglr
//    brbs 7, foo //look at interrupt bit in SREG
//    rjmp baz 

//-----------------------
//counting demo 
// foo:
//     ldi accum, 0x0f
//     ldi r20, 0x01
//     rjmp bar 
// bar:
//    //mul accum, r20
//    sub accum, r20 
//    sts PORTF, accum
//    rcall prelongerdelay
//    rjmp bar 
