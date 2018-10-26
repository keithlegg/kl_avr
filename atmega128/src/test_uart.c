

#define F_CPU 16000000UL


//UART STUFF
#define FOSC 16000000UL
#define BAUD 57600
#define MYUBRR FOSC/16/BAUD-1

#include <avr/io.h>
#include <util/delay.h>

#define BIT_ON 0x30 //ascii 1
#define BIT_OFF 0x31 //ascii 0


/***********************************************/

// page 183 of datasheet
void USART_Init( unsigned int ubrr)
{
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);

    //#if USE_2X
    UCSR0A |= (1<<U2X0); //DAMN DAMN DAMN - HOW DID I NOT FIND THIS SOONER????
    //#else
    //    UCSR0A &= ~(_BV(U2X0));
    //#endif

}

/***********************************************/

static uint8_t USART_receive(void)
{
    while (!(UCSR0A & (1 << RXC0))) {/*Busy wait.*/}
    return UDR0;
}

/***********************************************/

void USART_Transmit( char data )
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
}

/***********************************************/
void print_byte_16( uint16_t data){
   uint8_t i = 0;

   for (i=0; i<=15; i++) {
       //if ( !!(data & (1 << ii)) ){  // LSB
       if ( !!(data & (1 << (15 - i))) ){  // MSB
           USART_Transmit( BIT_OFF );
       }else{
           USART_Transmit( BIT_ON );
       }
    }
}

/***********************************************/
//debug experimental - cant seem to make this work!
void USART_tx_string( char data[]  )
{
    int i = 0; 
    while(data[i] != '\0')
    {
        UDR0 = data[i++];
        while (!(UCSR0A & (1 <<UDRE0)));
    }

    while (!(UCSR0A & (1 <<UDRE0)));
    UDR0 = '\0';
}

//*******************************************//


unsigned char y[20]; // in main
unsigned char len;

unsigned char uartrecieve(unsigned char *x, unsigned char size)
{
    unsigned char i = 0;

    if (size == 0) return 0;              // return 0 if no space

    while (i < size - 1) {                // check space is available (including additional null char at end)
        unsigned char c;
        while ( !(UCSR0A & (1<<RXC0)) );  // wait for another char - WARNING this will wait forever if nothing is received
        c = UDR0;
        if (c == '\0') break;             // break on NULL character
        x[i] = c;                         // write into the supplied buffer
        i++;
    }
    x[i] = 0;                             // ensure string is null terminated

    return i + 1;                         // return number of characters written
} 

//len = uartrecieve(y, 20);

//*******************************************//
void echo_uart(){
    uint8_t buf = USART_receive();
    print_byte(buf);
}

//*******************************************//

/*
//#define HEX_TO_DEC_ERROR 42 
unsigned char hex_to_dec(unsigned char in) 
{ 
   if(((in >= '0') && (in <= '9'))) return in-'0'; 
   in |= 0x20;
   if(((in >= 'a') && (in <= 'f'))) return in-'a' + 10; 
   return 42; 
}
*/


void USARTWriteStr(char *data) 
{ 
   while(*data){ 
      USART_Transmit(*data++); 
   }
}


int main (void)
{
   USART_Init(MYUBRR);


  //>>  I just need to figure out how to get the recieve to work in master mode
  //You send a dummy byte out (0x00 or 0xFF) and another one comes in from the slave, 
  //the same as when you are sending data out.
  //The slave can NEVER sent data out unless the master clock something out.
  //>>It is always the same "feature" of SPI that catches people:
  //You must make the SS pin an output whether you use it or not.


   //CPOL is set to one, SCK is high when idle, and if CPOL is set to zero, SCK is low when idle. 
   
   // So, there are four different ways of configuring the clock generation, which are known as 'SPI modes'. 
   // The following table summarizes the four SPI modes.

   // DDRB |= (1<<2)|(1<<3)|(1<<5);    // SCK, MOSI and SS as outputs

  
   /*
   //init SPI    
   DDRG = 0xff;
   DDRB = 0xff;
 
   //SPCR |= (1<<MSTR)|(1<<SPE)|(1<<CPHA)|(1<<CPOL);  // SPI Master, SPI Enable, Trailing edge
   SPCR |= (1<<MSTR)|(1<<SPE);  // SPI Master, SPI Enable
   SPSR |= SPI2X;

   */

   while(1){
       //USART_Transmit(0x42);

       //ECHO A BYTE BACK 
       USART_Transmit( USART_receive() );

       _delay_ms(100);
   }

}//main