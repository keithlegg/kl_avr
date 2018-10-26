
#define __SFR_OFFSET 0     // Needed to keep io.h from adding an incorrect offset to our port addresses
#include <avr/io.h>


#define LO(x) ((x) & 0xFF) 
#define HI(x) ((x)>>8) 

#define accum r16

.global main               // Main entry point

main:
   // Configure Port B for control signals
   ldi      accum,0xff      
   sts      DDRK,accum    

   // Put our stack at the top of internal SRAM
   ldi      accum,HI(RAMEND)
   out      SPH,accum
   ldi      accum,LO(RAMEND)
   out      SPL,accum

   rcall    bar


bar:
   ldi accum, 0xFF
   sts PORTK, accum        

   ldi accum, 0x00
   sts PORTK, accum

   rjmp bar   