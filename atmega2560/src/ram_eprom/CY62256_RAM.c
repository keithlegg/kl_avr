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

8/4/15 - 11:30PM- I think I witnessed RF noise on my control bus. I had weird errors in the data, 
 something inspired me to twist the 3 wires (WE_PIN, OE_PIN, CE_PIN 0x04) and it makes the problem go away!

--------------

The CY62256 is organized as 32K words by 8 bits. 

(CE OE LOW )
An active LOW write enable signal (WE) controls writing/reading operation of the memory. 
When CE and WE inputs are both LOW, data on the eight data input/output pins (I/O 0 through I/O 7 ) 
is written into the memory location addressed by the address present on the address pins 

( Reading = enabling the outputs, CE and OE active LOW, WE - HIGH. )
Reading the device is accomplished by selecting the device and enabling the outputs, CE and OE active LOW,
while WE remains inactive or HIGH. Under these conditions, the contents of the location addressed 
by the information on address pins are present on the eight data input/output pins.

The input/output pins remain in a high-impedance state unless the chip is selected, outputs are enabled,
and write enable (WE) is HIGH

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
      _____   _____
   A5 |1   |_|   28|Vcc
   A6 |2         27|WE - active low
   A7 |3         26|A4
   A8 |4         25|A3   
   A9 |5         24|A2  
   A10|6         23|A1 
   A11|7         22|OE  -active low
   A12|8         21|A0
   A13|8         20|CE - active low
   A14|8         19|IO7
   IO0|9         18|IO6
   IO1|10        17|IO5
   IO2|11        16|IO4
   GND|12        15|IO3
      -------------
*/

/***********************************************/
/***********************************************/

//PORTD - CONTROL BUS - bits 1-3
#define WE_PIN 0x01 
#define OE_PIN 0x02
#define CE_PIN 0x04

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
   
    if(x<256){
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
    PORTC = low_byte;
    PORTL = high_byte;

}


/***********************************************/

/*
Read procedure based on read cycle no. 2 from the CY62256 datasheet:

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

void read_ram(uint16_t address){
    
    DDRK = 0x00; 
    PORTD |= CE_PIN; 
    PORTK = 0x00; //clear data port
    set_address(address);

    PORTD |= WE_PIN; //WE high when reading 
    PORTD |= OE_PIN; 

    PORTD &= ~CE_PIN; 
    PORTD &= ~OE_PIN; 

    print_byte(PINK);
    
    PORTK = 0x00; //clear data port

    PORTD |= OE_PIN; 
    PORTD |= CE_PIN; 

}

/***********************************************/

/*
//http://theanine.io/projects/parallel_sram/

Write procedure based on write cycle no. 1 from the CY62256 datasheet:

    Set data port data direction to output (since we’re writing data)
    Make sure the chip is deselected (CE high)
    Put the address on the address bus
    Set WE and OE control lines to high before-hand
    Select the chip (CE low)
    Put the data on the data bus
    Set WE low (  This is when the chip inputs the data )
    Set WE high
    End the write cycle by deselecting the chip
*/

void write_ram(uint16_t address, uint8_t byte){
    
    DDRK = 0xff;
    PORTD |= CE_PIN;  
    set_address(address);

    PORTD |= WE_PIN; 
    PORTD |= OE_PIN;     

    PORTD &= ~CE_PIN; //CE low  

    PORTK = byte;
    PORTD &= ~WE_PIN;     

    PORTD |= WE_PIN;  

    //End the write cycle by deselecting the chip??
    PORTD |= CE_PIN;  

}


/***********************************************/

int main (void)
{
   //DDRB =0b00001000;  //arduino pin 11 
   //DDRB |= (1 << 3);  //arduino pin 11
   //DDRB = 0xff;      
   
   DDRC = 0xff;     
   DDRD = 0xff;  
   DDRL = 0xff; 
   //DDRK SET DYNAMICALLY 

   //DDRD |= (0x4); // PORTD!
   USART_Init(MYUBRR);
   



   //
   //------------
   
   //it is not addressing ODD address ONLY EVEN!
/*   write_ram(0,0xff);
   write_ram(1,0x1);
   write_ram(2,0x2);
   write_ram(3,0x4);
   write_ram(4,0x8);
   write_ram(6,0xff);   */
  
  
/* 
     write_ram(0, 0x00);
     write_ram(1, 0x02);
     write_ram(2, 0x04);
     write_ram(3, 0x08);
     write_ram(4, 0x00);
     write_ram(5, 0x00);
     write_ram(6, 0x00);
     write_ram(7, 0x00);
  //}*/

  uint16_t a = 0;
  for(a=0;a<100;a++){
      write_ram(a, a);
  }


  for(a=0;a<100;a++){
      read_ram(a);
  }
 //------------

} 

/***********************************************/

