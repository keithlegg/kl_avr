
#include <avr/io.h>


#define F_CPU 16000000UL //AVR Clock Speed in MHZ
#define FOSC 16000000    // Clock Speed

//UART STUFF
#define BAUD 57600
//#define MYUBRR FOSC/16/BAUD-1
#define UBBRVAL  FOSC/16/BAUD-1 //(((FOSC / (BAUD * 16UL))) - 1) 


//set baudrate2 
#define BAUD2 57600
#define UBBRVAL2  FOSC/16/BAUD2-1 //(((FOSC / (BAUD2 * 16UL))) - 1)


#define BIT_ON 0x30 //logic high
#define BIT_OFF 0x31 //logic low

#define TRUE                        1
#define FALSE                       0

#include <util/delay.h>



volatile uint8_t bufferReady   = FALSE;
volatile uint8_t bufIndex      =     0;
//volatile uint8_t uartRXBuf[RX_BUF_LEN];


/**********************************/

void USART_init_0(uint16_t ubrr)
{
    // Load upper and lower bytes of baud rate into the UBRR register
    UBRR0H = (uint8_t)(ubrr>>8); 
    UBRR0L = (uint8_t)ubrr;      

    // Enable transmitter and receiver.
    UCSR0B |= ((1 << RXEN0)|(1 << TXEN0));

    // Enable the USART RX interrupt.
    //UCSR1B |= (1 << RXCIE1); // Enable the USART Recieve Complete interrupt (USART_RXC)

}

/**********************************/

static void USART_transmit_0(uint8_t data)
{
    while (!(UCSR0A & (1 << UDRE0))) {/*Busy wait.*/}
    UDR0 = data;
}

/**********************************/
static uint8_t USART_receive_0(void)
{
    while (!(UCSR0A & (1 << RXC0))) {/*Busy wait.*/}
    return UDR0;
}

/**********************************/

static void USART_init_1(uint16_t ubrr)
{
    UBRR1H = (uint8_t)(ubrr>>8); 
    UBRR1L = (uint8_t)ubrr;     

    // Enable transmitter and receiver.
    UCSR1B |= ((1 << RXEN1)|(1 << TXEN1));
}

/**********************************/

static void USART_transmit_1(uint8_t data)
{
    while (!(UCSR1A & (1 << UDRE1))) {/*Busy wait.*/}
    UDR1 = data;
}

/**********************************/
static uint8_t USART_receive_1(void)
{
    while (!(UCSR1A & (1 << RXC1))) {/*Busy wait.*/}
    return UDR1;
}

/**********************************/
void send_string_0(char s[])
{
   int i =0;
   
   while (s[i] != 0x00)
   {
      USART_transmit_0(s[i]);
      i++;
   }
} 

/**********************************/

void send_string_0_2(char s[],int num)
{
   int i =0;
   
   for (i=0;i<num;i++)
   {
      if (s[i]!=0x00){
         USART_transmit_0(s[i]);
      }
   }
   USART_transmit_0('\n');

} 

/**********************************/


void print_byte( uint8_t data){
   uint8_t i = 0;

   for (i=0; i<=7; i++) {
       //if ( !!(data & (1 << ii)) ){  // LSB
       if ( !!(data & (1 << (7 - i))) ){  // MSB
           USART_transmit_0( BIT_OFF );
       }else{
           USART_transmit_0( BIT_ON );
       }
    }
    USART_transmit_0( 0xa ); //CHAR_TERM = new line  
    USART_transmit_0( 0xd ); //0xd = carriage return
}

/**********************************/

int main()
{
    // Set LED PB0-2 as output
    //DDRB |= (1 << 0)|(1 << 1)|(1 << 2); 
     
    DDRD = 0x00; //portD is 6502 data bus 
 
    //activate UART hardware/enable interrupts
    USART_init_0(UBBRVAL);    
    //USART_init_1(UBBRVAL2); //DONT USE 1 - ITS WORED TO 6502 DATA BUS!
   
    uint8_t y =0;

    while (TRUE)
    {

            USART_transmit_0( 0x42 );
            //send_string_0_2(uartRXBuf,64);
            //print_byte(PIND);
            //_delay_ms(100);

    }

    return 1;
}

/**********************************/




/*

FYI, the ATmega128 has some special differences you need to be aware of. Using the ATmega128 Rev. 

First look at section SPI Serial Programming Pin Mapping Table 127. Pin Mapping SPI Serial Programming.
 In some AVRs like the ATmega128 the USART0 pins are used for the ISP function, and the 
 SPI (do not confuse this with the ISP acronym) MOSI and MISO pins are not used. 
 In lots of other AVRs the SPI pins MOSI and MISO are used for the ISP function.

Then look at sections ATmega103 and ATmega128 Compatibility and ATmega103 Compatibility 
Mode for the M103C fuse information. The ATmega128 is shipped from ATMEL with the M103C
 fuse programmed (see Table 117. Extended Fuse Byte). So, every brand new ATmega128 ships
  from the factory with the M103C fuse programmed making your new chip default into an 
  ATmega103 (an old obsolete processor). If you purchased a circuit board with an ATmega128
   already soldered onto the board, then the board manufacturer may have already unprogrammed 
   the M103C fuse. You need to make sure the M103C fuse is unprogrammed if you really 
   want a working ATmega128 chip.

Since you are new to AVRs I just thought you should be aware of these special ATmega128 
differences. For example, if the M103C fuse is still programmed you could write perfectly
 good ATmega128 program code that will fail because the AVR is still in ATmega103 mode.


*/

/**********************************/

