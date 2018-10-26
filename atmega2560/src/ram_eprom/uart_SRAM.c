#include <avr/io.h>

#define F_CPU 16000000UL //AVR Clock Speed in MHZ
#define FOSC 16000000    // Clock Speed

//UART STUFF
#define BAUD 57600
#define MYUBRR FOSC/16/BAUD-1

#define BIT_ON 0x30 //logic high
#define BIT_OFF 0x31 //logic low

#include <util/delay.h>

/*
  Oct 21, 2015 (BTTF day!)

  I got this code working with a UM61512AK on the first try.
  Now I am adding a feature to dump the data to/from a serial port 


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
         UM61512A
       _____   _____
     NC|1   |_|   32|Vcc
     NC|2         31|A15
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
/*

   walk each location in RAM and send it over UART 

*/

void tx_bytes()
{
     int b = 0;

     for (b=0;b<65536;b++)
     {
         while ( !( UCSR0A & (1<<UDRE0)) );
         UDR0 = read_ram(b); 
     }//B-Y


}

/***********************************************/

void echo_uart(void){
    uint8_t buf = USART_receive();
    print_byte(buf);
}
/***********************************************/

#define CMD_ECHO 0x71         //q
//#define CMD_RST_SCAN 0x72     //r 
//#define CMD_START_SCAN 0x73   //s 

/********************/

void rx_command(void){
    while(1)
    {
        //listen for a command over UART  
        while (!(UCSR0A & (1 << RXC0))) {}//Busy wait.

        switch(UDR0)
        {
            case CMD_ECHO: //q
                echo_uart(); 
            break;
        }
    
    }//while
}

/***********************************************/

//send SRAM contents over uart to host machine 

//TODO
//  would be nice to have a "return 2 bytes from 16 bit" function 
//  also have a "make 16 from 2 bytes" function 

void tx_ram(uint16_t numbytes){
   uint16_t a = 0;
   // for(a=0;a<65535;a++){
   //     write_ram(a, a%256); //pack 16 bit value into 1 byte to send 
   // }
   
   for(a=0;a<numbytes;a++){
      //BASIC ERROR CHECKING - if two do not match we may have an error 
      //if( read_ram(a) != a%256){
          // USART_Transmit( read_ram(a) ); 
          // USART_Transmit( a%256 ); 
      // }

      print_byte( read_ram(a) ); 
   }

   // USART_Transmit( 0x43 ); 
   //tx_bytes(1);
   // // USART_Transmit( 0x42 );  
   //USART_Transmit( 0xa ); //CHAR_TERM = new line  
   //USART_Transmit( 0xd ); //0xd = carriage return

}

/***********************************************/
//fill local SRAM with data sent over uart from host machine 

//TODO
//  would be nice to have a "return 2 bytes from 16 bit" function 
//  also have a "make 16 from 2 bytes" function 

void rx_ram(void){
   uint16_t a = 0;

   //65535
   for(a=0;a<4;a++){ 
       write_ram(a, USART_receive() ); //pack 16 bit value into 1 byte to send 
   }
  

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

   //tx_ram();
   rx_ram();

   tx_ram(5);

} 

/***********************************************/


