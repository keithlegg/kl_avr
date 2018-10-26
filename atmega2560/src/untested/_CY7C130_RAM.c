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
//THIS IS NOT WORKING YET!!!
/***********************************************/

/*

The CY7C130/130A/CY7C131/131A/CY7C140 [1] and CY7C141
are high speed CMOS 1K by 8 dual-port static RAMs. Two ports
are provided permitting independent access to any location in
memory. The CY7C130/130A/ CY7C131/131A can be used as
either a standalone 8-bit dual-port static RAM or as a master
dual-port RAM in conjunction with the CY7C140/CY7C141 slave
dual-port device in systems requiring 16-bit or greater word
widths. It is the solution to applications requiring shared or
buffered data, such as cache memory for DSP, bit-slice, or multi-
processor designs.


Each port has independent control pins; chip enable (CE), write
enable (R/W), and output enable (OE). Two flags are provided
on each port, BUSY and INT. BUSY signals that the port is trying
to access the same location currently being accessed by the
other port. INT is an interrupt flag indicating that data is placed
in a unique location (3FF for the left port and 3FE for the right
port). An automatic power down feature is controlled indepen-
dently on each port by the chip enable (CE) pins.
The CY7C130/130A and CY7C140 are available in 48-pin DIP.
The CY7C131/131A and CY7C141 are available in 52-pin
PLCC, 52-pin Pb-free PLCC, 52-pin PQFP, and 52-pin Pb-free
PQFP.



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
                       CY_7C130
                    _____   _____
Activelow  -   CE L |1   |_|   48| VCC
Activelow  -  R/W L |2         47| CE R   - Active low
Activelow -  BUSY L |3         46| R/W R
Activelow -   INT L |4         45| BUSY R - Active low  
Activelow -    OE L |5         44| INT R  - Active low
               A0 L |6         43| OE R   - Active low 
               A1 L |7         42| A0 R
               A2 L |8         41| A1 R
               A3 L |9         40| A2 R 
               A4 L |10        39| A3 R
               A5 L |11        38| A4 R
               A6 L |12        37| A5 R
               A7 L |13        36| A6 R
               A8 L |14        35| A7 R
               A9 L |15        34| A8 R
             I/O0 L |16        33| A9 R   
             I/O1 L |17        32| I/O7 R  
             I/O2 L |18        31| I/O6 R   
             I/O3 L |19        30| I/O5 R 
             I/O4 L |20        29| I/O4 R 
             I/O5 L |21        28| I/O3 R 
             I/O6 L |22        27| I/O2 R 
             I/O7 L |23        26| I/O1 R 
              GND   |24        25| I/O0 R 
                     -------------
*/

/***********************************************/
/***********************************************/

//PORTD - CONTROL BUS - bits 1-3
#define RW_PIN 0x01 
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

    PORTD |= RW_PIN; //WE high when reading 
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

    PORTD |= RW_PIN; 
    PORTD |= OE_PIN;     

    PORTD &= ~CE_PIN; //CE low  

    PORTK = byte;
    PORTD &= ~RW_PIN;     

    PORTD |= RW_PIN;  

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

