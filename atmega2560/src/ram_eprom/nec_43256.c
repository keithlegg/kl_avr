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

10/19/15 - getting "back on the horse"

Oct 19, 11:45 pm - seems to work! 
  The first element is showing 00001001 instead of zero
  the second is showing zero 

8/4/15 - 11:30PM- I think I witnessed RF noise on my control bus. I had weird errors in the data, 
  something inspired me to twist the 3 wires (WE_PIN, OE_PIN, CS_PIN 0x04) and it makes the problem go away!


*/


/***********************************************/


/*

    (15 bit address bus)
    --------------------------------
    address (L)  (H)    | CONTROL 
           PORTC PORTL  | PORTD
           [0-7] [8-13] | [1-3]  
    --------------------------------
    PORTL - address bus low   (7 bit)
    PORTC - address bus high  (7 bit)
    PORTK - data bus
    PORTD - control bus       (3 bits)


*/

/***********************************************/

/*
      NEC pd43256

      _____   _____
   A14|1   |_|   28|Vcc
   A12|2         27|/WE - active low
    A7|3         26|A13
    A6|4         25|A8   
    A5|5         24|A9  
    A4|6         23|A11 
    A3|7         22|/OE -active low
    A2|8         21|A10
    A1|9         20|/CS -active low
    A0|10        19|IO8
   IO1|11        18|IO7
   I02|12        17|IO6
   IO3|13        16|IO5
   GND|14        15|IO4         
      -------------
*/

/***********************************************/
/***********************************************/

//PORTD - CONTROL BUS - bits 1-3
#define WE_PIN 0x01 
#define OE_PIN 0x02
#define CS_PIN 0x04  //CE is CS 

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


void test_ports(void){
    int a=0;
    for(a=0;a<8;a++){
       PORTL = idx_to_byte(a);
       _delay_ms(100);
    }
    for(a=0;a<8;a++){
       PORTC = idx_to_byte(a);
       _delay_ms(100);
    }       
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
    

    //wasted hours - switched these out of desperation and NOW it works?
    //symptom was - repeated addressing, 11,22,33 instead of 123456
    //I dont think these are ACTUALLY REVERSED - just realized i was wiring the byte leds 
    //with MSB on the RIGHT not LEFT so I think I was confused still, it seems to work
    
    PORTL = low_byte;
    PORTC = high_byte;
}


/***********************************************/

/*

READS when CS, OE and LOW and WE is HIGH 
in read cycle, WE should be fixed to high level

 from the CY62256 datasheet (not 43256):

    Set data port data direction to input (since we’re reading data)
    Make sure the chip is deselected (CE high)
    Put the address on the address bus
    Set WE and OE to high before-hand
    Select the chip (CE low)
    Set OE low (This is when the chip outputs the data)
    
    Read from the data bus
    Set OE high
    End the write cycle by deselecting the chip

*/

uint8_t read_ram(uint16_t address){
    
    uint8_t out = 0x00;

    DDRK = 0x00; 
    PORTD |= CS_PIN; 
    PORTK = 0x00; //clear data port
    set_address(address);

    PORTD |= WE_PIN; //WE high when reading 
    PORTD |= OE_PIN; 

    PORTD &= ~CS_PIN; 
    PORTD &= ~OE_PIN; 

    out = PINK;
    
    PORTK = 0x00; //clear data port

    PORTD |= OE_PIN; 
    PORTD |= CS_PIN; 

    return out;
}

/***********************************************/

void print_ram(uint16_t address){
    
    DDRK = 0x00; 
    PORTD |= CS_PIN; 
    PORTK = 0x00; //clear data port
    set_address(address);

    PORTD |= WE_PIN; //WE high when reading 
    PORTD |= OE_PIN; 

    PORTD &= ~CS_PIN; 
    PORTD &= ~OE_PIN; 

    print_byte(PINK);
    
    PORTK = 0x00; //clear data port

    PORTD |= OE_PIN; 
    PORTD |= CS_PIN; 

}

/***********************************************/

/*
 WRITE when CS and WE are LOW 
 CS or WE should be fixed to high level during address transition

 -------

//http://theanine.io/projects/parallel_sram/
from the CY62256 datasheet (not 43256):

    Set data port data direction to output (since we’re writing data)
    Make sure the chip is deselected (CS high)
    Put the address on the address bus
    Set WE and OE control lines to high before-hand
    Select the chip (CS low)
    Put the data on the data bus
    Set WE low (  This is when the chip inputs the data )
    Set WE high
    End the write cycle by deselecting the chip
*/

void write_ram(uint16_t address, uint8_t byte){
    
    DDRK = 0xff;
    PORTD |= CS_PIN;  
    PORTD |= WE_PIN; 
    PORTD |= OE_PIN;     

    set_address(address);

    PORTD &= ~CS_PIN;  

    PORTK = byte;
    //_delay_ms(5);
    PORTD &= ~WE_PIN;     
    //_delay_ms(5);

    PORTD |= WE_PIN;  

    //End the write cycle by deselecting the chip??
    //PORTD |= CS_PIN;  

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
   for(a=0;a<10;a++){
       write_ram(a, a);
       //set_address(a);
   }

/*   write_ram(0, 0);
   write_ram(1, 255);
   write_ram(2, 2);
   write_ram(3, 180);
   write_ram(4, 4);*/

   for(a=0;a<10;a++){
      //read_ram(a);
      print_ram(a);
   }


} 

/***********************************************/

