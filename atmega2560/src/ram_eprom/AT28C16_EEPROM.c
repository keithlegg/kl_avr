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

 The AT28C16 is accessed like a Static RAM.  When CE and OE are low and WE is high, the data stored
 at the memory location determined by the address pins is asserted on the outputs. 
 The outputs are put in a high impedance state whenever CE  or OE is high. This dual line
 control gives designers increased flexibility in preventing bus contention

*/

/***********************************************/

/*

    (10 bit address bus)
    --------------------------------
    address (L)  (H)    | CONTROL 
           PORTL PORTC  | PORTC
           [0-7] [8-11] | [12-15]
    --------------------------------

    PORTC - address bus high (only lower2 making 11 bits /2047 values)
    PORTC - control bus      (higher 5 bits)
    PORTK - data bus

*/

/***********************************************/

/*
      ___   ___
   A7|   |_|   |Vcc
   A6|         |A8
   A5|         |A9
   A4|         |WE  -active low
   A3|         |OE  -active low
   A2|         |A10
   A1|         |CE  -active low
   A0|         |IO7
  IO0|         |IO6
  IO1|         |IO5
  IO2|         |IO4
  GND|         |IO3
      ---------
*/

/***********************************************/
/***********************************************/

//PORTC - CONTROL BUS - bits 3-5
#define WE_PIN 0x8
#define OE_PIN 0x10
#define CE_PIN 0x20
/***********************************************/

void USART_Init( unsigned int ubrr)
{
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    /*Enable receiver and transmitter */
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);

}


/***********************************************/

static uint8_t USART_receive(void)
{
    while (!(UCSR0A & (1 << RXC0))) {/*Busy wait.*/}
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
    
    PORTL = low_byte;
    // clear lower 3 bits of portc, (upper 5 are control bus)
    PORTC &= ~ 0x7; 
    //set lower 2 bits of portc
    PORTC |= (high_byte &= 0x7); //clamp it to 11 bit 
}


/***********************************************/
//When CE and OE are low and WE is high,data at address pins is asserted on the outputs.

void read_eeprom(uint16_t address){
    
    //set address bus
    set_address(address);
 
    //set control bus 
    PORTC |= WE_PIN; //we high
    PORTC &= ~OE_PIN; //oe low
    PORTC &= ~CE_PIN; //ce low

    //read port and send byte over serial port 
    print_byte(PINK);

    //hang out and let user read incoming data
    _delay_ms(1000);

}


/***********************************************/

int main (void)
{
  
   DDRL = 0xff; 
   DDRC = 0xff;     
   DDRK = 0x00; 

   USART_Init(MYUBRR);
   
   int a = 0;

   while (1)
   {
       for (a=0;a<10;a++){
           read_eeprom(a);
       }
   }


} 




/***********************************************/

/***********************************************/



//used to test wiring of data/address busses
/*
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
*/


/***********************************************/
/*
// 16 to 8 bit 
uint8_t Data_enkoder_buffer_8;
uint16_t Data_enkoder_buffer_16;

Data_enkoder_buffer_8 = ReadByteSPI(0xff);
Data_enkoder_buffer_16 = (ReadByteSPI(0xff)<<8);
Data_enkoder_1 = (Data_enkoder_buffer_16 | Data_enkoder_buffer_8);
*/
/***********************************************/
/*
out16bit = (highI2C << 8);
out16bit = (highI2C << 7) | lowI2C;
//outputs correctly, I mean it does what is expected, not that result is what I am after.

//But when I use code:
out16bit = (highI2C << 8) | lowI2C;
??
*/


