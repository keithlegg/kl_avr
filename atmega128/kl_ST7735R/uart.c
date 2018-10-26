
#include <avr/io.h>

#define BIT_ON 0x30 //ascii 1
#define BIT_OFF 0x31 //ascii 0


//UART STUFF
#define FOSC 16000000UL
#define BAUD 57600
#define MYUBRR FOSC/16/BAUD-1


void send_txt_byte( uint8_t data, uint8_t use_newline){
   uint8_t i = 0;

   for (i=0; i<=7; i++) {
       //if ( !!(data & (1 << ii)) ){  // LSB
       if ( !!(data & (1 << (7 - i))) ){  // MSB
           USART_Transmit( BIT_OFF );
       }else{
           USART_Transmit( BIT_ON );
       }
    }

    if(use_newline!=0){
        USART_Transmit( 0xa ); //CHAR_TERM = new line  
        USART_Transmit( 0xd ); //0xd = carriage return
    }
}

/***********************************************/

// page 183 of datasheet
void USART_Init(void)
{
    UBRR0H = (unsigned char)(MYUBRR>>8);
    UBRR0L = (unsigned char)MYUBRR;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);

    //#if USE_2X
    UCSR0A |= (1<<U2X0); //DAMN DAMN DAMN - HOW DID I NOT FIND THIS SOONER????
}

/***********************************************/

uint8_t USART_receive(void)
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

/***********************************************/
void USARTWriteStr(char *data) 
{ 
   while(*data){ 
      USART_Transmit(*data++); 
   }
}


/***********************************************/


//unsigned char y[20]; // in main
//unsigned char len;

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
