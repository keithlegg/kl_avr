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
   this code seems to work-ish with address line 17 wired LOW so I dont have to allocate a 
   32bit address variable for one more bit.

   I cant seem to get the ram to store. THe refresh pin #1 is some annomaloy, 
   I cant find it on a data sheet anymore. DOnt know where I found it in the first place

*/

/***********************************************/

/*
                       DS1245YAB
                      _____   _____
    active low- RFRSH|1   |_|   32|Vcc
                  A16|2         31|A15
                  A14|3         30|CE2 
                  A12|4         29|R/W 
                   A7|5         28|A13  
                   A6|6         27|A8 
                   A5|7         26|A9
                   A4|8         25|A11
                   A3|9         24|/OE  -active low 
                   A2|10        23|A10
                   A1|11        22|/CE1 - active low 
                   A0|12        21|IO7
                  IO0|13        20|IO6
                  IO1|14        19|IO5
                  IO2|15        18|IO4
                  GND|16        17|IO3
                     -------------
*/

/***********************************************/


/*

    (17 bit address bus)
    --------------------------------
    address (L)  (H)          | CONTROL 
           PORTC PORTL  PORT? | PORTD
           [0-7] [8-13] [1]   | [1-3]  
    --------------------------------
    PORTL - address bus low   (8 bit)
    PORTC - address bus high  (8 bit)
    PORT? - address bus high  (1 bit) - I hardwired this for LOW testing 
    PORTK - data bus          (8 bit)
    PORTD - control bus       (3 bits)


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
/*
 17 bit address! 

*/

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
/*
READ MODE
The DS1245 executes a read cycle whenever WE (Write Enable) is inactive (high) and CE (Chip Enable)
and OE (Output Enable) are active (low). The unique address specified by the 17 address inputs (A0-A16)
defines which of the 131,072 bytes of data is to be accessed. Valid data will be available to the eight
data output drivers within t ACC (Access Time) after the last address input signal is stable, providing
that CE and OE (Output Enable) access times are also satisfied. If OE and CE access times are not satisfied,
then data access must be measured from the later occurring signal ( CE or OE ) and the limiting parameter
is either t CO for CE or t OE for OE rather than address access.
*/

uint8_t read_ram(uint16_t address){
    
    uint8_t out = 0x00;

    DDRK = 0x00; 
    PORTK = 0x00; //clear data port
    
    PORTD |= CS_PIN; 
    PORTD |= OE_PIN; 
    PORTD |= WE_PIN; //WE high when reading 

    set_address(address);

    PORTD &= ~CS_PIN; 
    PORTD &= ~OE_PIN; 
    
    _delay_ms(1);

    out = PINK;

    return out;
}

/***********************************************/

/*
The DS1245 executes a write cycle whenever the WE and CE signals are active (low) after address
inputs are stable. The later occurring falling edge of CE or WE will determine the start of the write cycle.
The write cycle is terminated by the earlier rising edge of CE or WE . All address inputs must be kept
valid throughout the write cycle. WE must return to the high state for a minimum recovery time (t WR )
before another cycle can be initiated. The OE control signal should be kept inactive (high) during write
cycles to avoid bus contention. However, if the output drivers are enabled ( CE and OE active) then WE
will disable the outputs in t ODW from its falling edge.
*/

void write_ram(uint16_t address, uint8_t byte){
    
    DDRK = 0xff;
    PORTD |= OE_PIN;  

    PORTD |= CS_PIN;  
    PORTD |= WE_PIN; 

    set_address(address);
    
    PORTK = byte;
    PORTD &= ~CS_PIN;  
    PORTD &= ~WE_PIN;   
    
    _delay_ms(1);

    PORTD |= WE_PIN;  


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

  for(a=0;a<20;a++){
       write_ram(a, a);
   }


   for(a=0;a<20;a++){
      print_byte( read_ram(a) ); 
   }


} 

/***********************************************/



