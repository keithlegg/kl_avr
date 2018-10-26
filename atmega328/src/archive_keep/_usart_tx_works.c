#include <avr/io.h>
#include <util/delay.h>

//page 111 in datasheet 


#define FOSC 16000000 // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1



unsigned char snippy ;



//*************************************************************
//USART_init: initializes the USART system
//*************************************************************

//page 183 of datasheet
void USART_Init( unsigned int ubrr)
{

	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	/*Enable receiver and transmitter */
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 2stop bit */
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}



void USART_Transmit( unsigned char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}



void USART_tx_string( char *data )
{
while ((*data != '\0'))
   {
      while (!(UCSR0A & (1 <<UDRE0)));
      UDR0 = *data;
      data++;
   }   
}






int main(){
  DDRB = 0xFF; 
  while (1)
  {
    
     //USART_Init(103);
     USART_Init(MYUBRR);


     USART_Transmit( 0x41);
     _delay_ms(400);
     USART_Transmit( 0x42);
     _delay_ms(200);    


  }

 return 1;
}



