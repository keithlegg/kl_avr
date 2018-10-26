
#define __SFR_OFFSET 0     // Needed to keep io.h from adding an incorrect offset to our port addresses
#include <avr/io.h>


#define LO(x) ((x) & 0xFF) 
#define HI(x) ((x)>>8) 

#define accum r16


.equ   baud, 9600          //;Baud rate 
.equ   fosc, 16000000      //;Crystal frequency 


.global main               // Main entry point


//----------------------------//
main:
    // Configure Port B for control signals
    ldi      accum,0xff      
    sts      DDRF,accum    //PORTF needs to be sts not out!!

    // Put our stack at the top of internal SRAM
    ldi      accum,HI(RAMEND)
    out      SPH,accum
    ldi      accum,LO(RAMEND)
    out      SPL,accum

    //////
    
    //rcall USART_Init0
    rcall USART_Init1
    rcall loop

//----------------------------//

// reset: 
//    rcall USART_Init
//    ldi      r16, 0x0d   //;Print carriage return
//    rcall TX1 
//    ldi      r16, 0x0a   //;Print line feed
//    rcall TX1 
//    ldi      r16, '>'    //;Print first char,
//    rcall TX1 
//    rjmp loop 
//    //rjmp reset

//----------------------------//
USART_Init0: 

    //C CODE 
    //UBRR0H = (unsigned char)(ubrr>>8);
    //UBRR0L = (unsigned char)ubrr;
    //UCSR0B = (1<<RXEN0)|(1<<TXEN0);

   ldi r17, HI(fosc/(16*baud)-1) 
   ldi r16, LO(fosc/(16*baud)-1) 

   //// Set baud rate 
   sts UBRR0H, r17 
   sts UBRR0L, r16 
   
   //// Enable receiver and transmitter 
   ldi r16, (1<<RXEN0)|(1<<TXEN0) 
   sts UCSR0B,r16 

   //// SEEMS TO NOT WORK ! - Set frame format: 8data, 1stop bit 
   //ldi r16, (0<<USBS0)|(3<<UCSZ00) 
   //sts UCSR0C,r16 
   ret 


//----------------------------//
USART_Init1: 

   ldi r17, HI(fosc/(16*baud)-1) 
   ldi r16, LO(fosc/(16*baud)-1) 

   //// Set baud rate 
   sts UBRR1H, r17 
   sts UBRR1L, r16 
   
   //// Enable receiver and transmitter 
   ldi r16, (1<<RXEN1)|(1<<TXEN1) 
   sts UCSR1B,r16 

   ret 

//----------------------------//

//ECHO SERIAL DATA
loop: 
   
   ldi r16, 0x47   
   rcall TX1 

   rjmp loop 


//----------------------------//
//untested
RX1: 
   //; Wait for data to be received 
   lds r17, UCSR1A 
   sbrs r17, RXC1 
   rjmp RX1 

   //; Get and return received data from buffer 
   lds r16, UDR1 
   ret 

//----------------------------//

TX1: 
   //; Wait for empty transmit buffer 
   lds r17, UCSR1A          //;Load into R17 from SRAM UCSR0A          
   sbrs r17, UDRE1          //;Skip next instruction If Bit Register is set 
   rjmp TX1 
   //; Put data (r0) into buffer, sends the data 
   sts UDR1, r16 
   ret 


