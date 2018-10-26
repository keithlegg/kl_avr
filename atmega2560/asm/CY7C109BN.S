/***********************************************/
/*
    (17 bit address bus) I wired 17 high to test 
    --------------------------------
    address (L)  (H)    | CONTROL 
           PORTC PORTL  | PORTD
           [0-7] [8-13] | [1-3]  
    --------------------------------
    PORTL - address bus low   (7 bit)
    PORTC - address bus high  (7 bit)
    PORTK - data bus
    PORTD - control bus       (3-4 bits)
-----------------------------------------------------
  
         CY7C109BN
       _____   _____
     NC|1   |_|   32|Vcc
    A16|2         31|A15
    A14|3         30|CE2 - WIRE TO VCC!
    A12|4         29|/WE  -active low   
     A7|5         28|A13  
     A6|6         27|A8 
     A5|7         26|A9
     A4|8         25|A11
     A3|9         24|/OE  -active low 
     A2|10        23|A10
     A1|11        22|/CE1 - active low 
     A0|12        21|IO8
    IO1|13        20|IO7
    IO2|14        19|IO6
    IO3|15        18|IO5
    GND|16        17|IO4
       -------------
*/
/***********************************************/



#define __SFR_OFFSET 0     // Needed to keep io.h from adding an incorrect offset to our port addresses
#include <avr/io.h>


#define LO(x) ((x) & 0xFF) 
#define HI(x) ((x)>>8) 

#define accum r16

#define rxbuf r19
#define txbuf r20


.equ   baud, 115200        //;Baud rate 
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
    
    rcall USART_Init0
    
    //rcall USART_Init1
    rcall loop



    
//----------------------------//
predelay:
    push r16
    ldi r16, 255
delay:
    dec r16
    brne delay
    pop r16
    ret 
   
//----------------------------//
prelongdelay:
    push r16
    ldi r16, 255
longdelay:
    rcall predelay
    dec r16
    brne longdelay
    pop r16
    ret 
   
//----------------------------//
prelongerdelay:
    push r16
    ldi r16, 20
longerdelay:
    rcall prelongdelay
    dec r16
    brne longerdelay
    pop r16
    ret 


//----------------------------//
// void print_byte( uint8_t data){
//    uint8_t i = 0;
//    for (i=0; i<=7; i++) {
//        //if ( !!(data & (1 << ii)) ){  // LSB
//        if ( !!(data & (1 << (7 - i))) ){  // MSB
//            USART_Transmit( BIT_OFF );
//        }else{
//            USART_Transmit( BIT_ON );
//        }
//     }
//     USART_Transmit( 0xa ); //CHAR_TERM = new line  
//     USART_Transmit( 0xd ); //0xd = carriage return
// }


//----------------------------//
//ECHO SERIAL DATA
loop: 
   
   //ldi r16, 0x36   
   //rcall TX0 
   //rcall TX1 

   ldi accum, 8
   rcall print_byte

   rjmp loop 


//----------------------------//
print_byte:
 
    //txbuf
    mov txbuf, accum  //accum
    //ldi txbuf, 0xff
    rcall TX0

    rcall prelongerdelay
    
    dec accum
    brne print_byte
    ret


//----------------------------//
// uint8_t set_address(uint16_t address){
//     uint8_t low_byte  = 0;
//     uint8_t high_byte = 0;
//     if(address<256){
//         high_byte = 0; 
//         low_byte  = address; 
//     }        
//     else{
//         //high byte, shift all bits 8 places right
//         high_byte = (uint8_t)(address >> 8);
//         //low byte, clear the high byte
//         low_byte = (uint8_t)(address & 0x00FF);
//     }
//     PORTL = low_byte;
//     PORTC = high_byte;
// }

//----------------------------//
// uint8_t read_ram(uint16_t address){
//     uint8_t out = 0x00;
//     DDRK = 0x00; 
//     PORTD |= CS_PIN; 
//     PORTK = 0x00; //clear data port
//     set_address(address);
//     PORTD |= WE_PIN; //WE high when reading 
//     PORTD |= OE_PIN; 
//     PORTD &= ~CS_PIN; 
//     PORTD &= ~OE_PIN; 
//     out = PINK;
//     PORTK = 0x00; //clear data port
//     PORTD |= OE_PIN; 
//     PORTD |= CS_PIN; 
//     return out;
// }


//----------------------------//
// void write_ram(uint16_t address, uint8_t byte){
//     DDRK = 0xff;
//     PORTD |= CS_PIN;  
//     PORTD |= WE_PIN; 
//     PORTD |= OE_PIN;     
//     set_address(address);
//     PORTD &= ~CS_PIN;  
//     PORTK = byte;
//     PORTD &= ~WE_PIN;     
//     PORTD |= WE_PIN;  
// }



//----------------------------//
USART_Init0: 

   ldi r17, HI(fosc/(16*baud)-1) 
   ldi r16, LO(fosc/(16*baud)-1) 

   //// Set baud rate 
   sts UBRR0H, r17 
   sts UBRR0L, r16 
   
   //// Enable receiver and transmitter 
   ldi r16, (1<<RXEN0)|(1<<TXEN0) 
   sts UCSR0B,r16 

   //// SEEMS TO NOT WORK ! - Set frame format: 8data, 1stop bit 
   ldi r16, (0<<USBS0)|(3<<UCSZ00) 
   sts UCSR0C,r16 
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
//untested
RX0: 
   //; Wait for data to be received 
   lds r17, UCSR0A 
   sbrs r17, RXC0 
   rjmp RX0 

   //; Get and return received data from buffer 
   lds r16, UDR0 
   ret 

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

//wont work - I suspect bad soldering on test board
TX0: 
    //; Wait for empty transmit buffer 
    lds r17, UCSR0A          //;Load into R17 from SRAM UCSR0A          
    sbrs r17, UDRE0          //;Skip next instruction If Bit Register is set 
    rjmp TX0 
    //; Put data (r0) into buffer, sends the data 
    sts UDR0, txbuf 
   
    ret 

//----------------------------//

TX1: 
   //; Wait for empty transmit buffer 
   lds r17, UCSR1A          //;Load into R17 from SRAM UCSR0A          
   sbrs r17, UDRE1          //;Skip next instruction If Bit Register is set 
   rjmp TX1 
   //; Put data (r0) into buffer, sends the data 
   sts UDR1, txbuf 
   ret 


