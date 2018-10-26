#include <avr/io.h>

#define F_CPU 16000000UL //AVR Clock Speed in MHZ
#define FOSC 16000000    // Clock Speed

//UART STUFF
#define BAUD 57600
#define MYUBRR FOSC/16/BAUD-1

#define BIT_ON 0x30 //logic high
#define BIT_OFF 0x31 //logic low

#include <util/delay.h>

/***********************************************/

/*

  test of 16bit address decodinging circuitry
  We only need to use one byte - the MSB since anythng under bit 11 is not relavant.

  CS lines ARE USUALLY ACTIVE LOW meaning that it is ON if the line is OFF!!
  READ THE DATASHEET AND CHECK EVERYTHING THREE TIMES!

  For accessing sram there is a phi2 clock output that needs to be simulated.
  This is because the 6502 only writes to ram when the clock is high. We have to
  simulate that in code or hardwire it. I chose to hardwire and that works fine.

  -----------------------------

  Memory map - taken from loom communications 6502 SBC and Garth Wilsons wonderful pages. 
  // http://www.loomcom.com/projects/6502/
  // http://wilsonminesco.com/6502primer/addr_decoding.html

  $0-$7FFF - 111111111111111   - 32767 - SRAM
  $8000    - 1000000000000000  - 32768 - VIA  
  $8800    - 1000100000000000  - 34816 - ACIA
  $C000    - 1100000000000000  - 49152 - ROM

*/

/***********************************************/

/*
    # WIRING SETUP 

    PORTL - address low byte 
    PORTC - address high byte  

*/

/***********************************************/
/***********************************************/

void USART_Init( unsigned int ubrr)
{
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);

}


/***********************************************/

static uint8_t USART_receive(void)
{
    while (!(UCSR0A & (1 << RXC0))) {}
    return UDR0;
}

/***********************************************/

void USART_Transmit( unsigned char data )
{
  while ( !( UCSR0A & (1<<UDRE0)) );
  UDR0 = data;
}


/***********************************************/

void print_byte( uint8_t data){
   uint8_t i = 0;

   for (i=0; i<=7; i++) {
       //if ( !!(data & (1 << ii)) ){  // LSB
       if ( !!(data & (1 << (7 - i))) ){  // MSB
           USART_Transmit( BIT_OFF );
       }else{
           USART_Transmit( BIT_ON );
       }
    }
    USART_Transmit( 0xa ); //CHAR_TERM = new line  
    USART_Transmit( 0xd ); //0xd = carriage return
}

/***********************************************/
uint8_t idx_to_byte(uint8_t idx){
    return (1 << idx);
} 

/***********************************************/
//address is 10 bit actually, - we split it into two 8's and clamp it at 1023 
uint8_t set_address(uint16_t address){
   
    uint8_t low_byte  = 0;
    uint8_t high_byte = 0;
    
    if(address<256){
        high_byte = 0; 
        low_byte  = address; 
    }        
    else{
        //high byte, shift all bits 8 places right
        high_byte = (uint8_t)(address >> 8);
        //low byte, clear the high byte
        low_byte = (uint8_t)(address & 0x00FF);
    }
    
    PORTC = high_byte;
    PORTL = low_byte;

}

/***********************************************/

int main (void)
{
  
   
   DDRC = 0xff;     
   DDRL = 0xff; 

   /***************/

   // if you want to run through all address permutations    
/*   while(1){
      PORTC = 0x00;
      PORTL = 0x00;
      uint16_t a = 0;
      for(a=0;a<65535;a++){
         set_address(a);
         _delay_ms(1);
      }
   }*/

   /***************/

  // if you want to run through a range of addresses   
   while(1){
      PORTC = 0x00;
      PORTL = 0x00;
      uint16_t a = 0;
      for(a=32760;a<32770;a++){
         set_address(a);
         _delay_ms(400);
      }
   }

   /***************/


/*     int del = 600;
     while(1){

       set_address(32767); //$7FFF - top of RAM address
       _delay_ms(del*3); //wait longer to indicate the cyle began

       set_address(32768);//$8000 - bottom of VIA address
       _delay_ms(del);
       
       set_address(34816);//$8800 - bottom of ACIA address
       _delay_ms(del);

       set_address(49152);//$C000 - botton of ROM 
       _delay_ms(del);

     }*/

     

} 

/***********************************************/

