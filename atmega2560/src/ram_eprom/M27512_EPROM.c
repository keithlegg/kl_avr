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
Read Mode:

  See below.


Standby Mode: 

  The M27512 has a standby mode which reduces the maximum active power current from 125mA to
  40mA. The M27512 is placed in the standby mode by applying a TTL high signal to the E input. When
  in the standby mode, the outputs are in a high impedance state, independent of the GV PP input.

*/

/***********************************************/

/*

    (15 bit address bus)
    --------------------------------
    address (L)  (H)    | NO CONTROL NEEDED - TIE PINS LOW
           PORTL PORTC  | 
           [0-7] [8-13] |   
    --------------------------------
    PORTL - address bus low   (7 bit)
    PORTC - address bus high  (7 bit)
    PORTK - data bus



*/

/***********************************************/

/*
          M27512
       _____   _____
   A15 |1   |_|   28| Vcc
   A12 |2         27| A14
   A7  |3         26| A13
   A6  |4         25| A8   
   A5  |5         24| A9  
   A4  |6         23| A11 
   A3  |7         22| GVpp - Ouput enable, active low
   A2  |8         21| A10
   A1  |9         20| E    - Chip Enable, active low
   A0  |10        19| Q7
   Q0  |11        18| Q6
   Q1  |12        17| Q5
   Q2  |13        16| Q4
   GND |14        15| Q3
       -------------
*/

/***********************************************/
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
    PORTL = low_byte;
    PORTC = high_byte;

}


/***********************************************/

/*

Read Mode:

  The M27512 has two control functions, both of which must be logically active in order to obtain
  data at the outputs. 

  Chip Enable (E) is the power control and should be used for device selection.
  Output Enable (G) is the output control and should be used to gate data to the output pins, 
  independent of device selection. Assuming that the addresses are stable, address access time (t AVQV )
  is equal to the delay from E to output (t ELQV ). 

  Data is available at the outputs after delay of t GLQV from
  the falling edge of G, assuming that E has been low and the addresses have been stable for at least  
  t AVQV -t GLQV .

*/

void read_eprom(uint16_t address){
    
    DDRK = 0x00; 
    PORTK = 0x00; //clear data port
    set_address(address);
    print_byte(PINK);
    
    PORTK = 0x00; //clear data port

}



/***********************************************/

int main (void)
{
   //DDRB =0b00001000;  //arduino pin 11 
   //DDRB |= (1 << 3);  //arduino pin 11
   //DDRB = 0xff;      
   
   DDRC = 0xff;     
   DDRL = 0xff; 
   DDRK = 0x00;  

   USART_Init(MYUBRR);
 
   int a = 0;

  for(a=0;a<100;a++){
      read_eprom(a);
  }
 //------------

} 

/***********************************************/

