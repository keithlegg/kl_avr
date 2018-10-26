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
                    2114
                 _____   _____
             A6 |1   |_|    18| VCC
             A5 |2          17| A7
             A4 |3          16| A8
             A3 |4          15| A9 
             A0 |5          14| DQ1
             A1 |6          13| DQ2
             A2 |7          12| DQ3
 active low -/CS|8          11| DQ4
             GND|9          10| /WE   - Active low                     
                 -------------
*/

  ///////////////////

/*

PORTB 0-7 = ADDRESS 0-7
PORTC 0-1 = ADDRESS 8-9 
PORTC 2-3 = CONTROL BUS - CS AND WE 
PORTC 4-5 = G/B LEDS 
PORTD 2-5 = DATA BUS 

*/
  ///////////////////
/*

  BOX HEADERS ON PCB 
 
  LEFT 10 PIN HEADER - MAPPED TO 2114 DIP  (TOP)
   ______________
  |10-12-14-16-18|
  |NC-11-13-15-17|
  ----------------


  RIGHT 10 PIN HEADER - MAPPED TO 2114 DIP  (TOP)
   ___________
  |1 -3-5-7-9|
  |NC-2-4-6-8|
  ------------


*/



/***********************************************/
/***********************************************/

//PORTD - CONTROL BUS - bits 1-3
#define WE_PIN 0x01
#define CS_PIN 0x02

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

void print_byte( uint8_t data, uint8_t numbits){
   uint8_t i = 0;

    //numbits = bits-1 because of zero index
   for (i=0; i<=7; i++) {
       //optional clamp to only send N number LSB's in the byte 
       if(i>=numbits){
           //if ( !!(data & (1 << ii)) ){  // LSB
           if ( !!(data & (1 << (7 - i))) ){  // MSB
               USART_Transmit( BIT_OFF );
           }else{
               USART_Transmit( BIT_ON );
           }
       }
    }
    USART_Transmit( 0xa ); //CHAR_TERM = new line  
    USART_Transmit( 0xd ); //0xd = carriage return
}

/***********************************************/
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
    
    PORTL = low_byte;
    PORTC = high_byte;
}


/***********************************************/
//Read when CS LOW and WE HIGH 
void read_ram(uint16_t address){
    
    DDRK = 0x00; 
    PORTD |= CS_PIN; 
    PORTD |= WE_PIN; //must remain high during read 

    PORTK = 0x00; //clear data port
    set_address(address);

    PORTD &= ~CS_PIN; 
    _delay_ms(2);

    print_byte(PINK, 4);
    
    //PORTK = 0x00; //clear data port
    //PORTD |= CS_PIN; 

}

/***********************************************/
//Write when CS LOW and WE LOW 
void write_ram(uint16_t address, uint8_t byte){
    
    DDRK = 0xff;
    PORTD |= CS_PIN;  
    PORTD |= WE_PIN; 

    set_address(address);

    PORTD &= ~CS_PIN;  
    PORTD &= ~WE_PIN;  

    PORTK = byte;

}


/***********************************************/

int main (void)
{
  
   DDRC = 0xff;     
   DDRD = 0xff;  
   DDRL = 0xff; 
   //DDRK SET DYNAMICALLY 

   //DDRD |= (0x4); // PORTD!
   USART_Init(MYUBRR);
  
  uint16_t a = 0;

  for(a=0;a<50;a++){
     write_ram(a, a);
  }

  for(a=0;a<50;a++){
      read_ram(a);
  }

 //------------

} 

/***********************************************/

