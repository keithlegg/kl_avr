#include <avr/io.h>
#include <util/delay.h>

//page 111 in datasheet 


//16000000UL

#define F_CPU 16000000UL
//#define UBRR_2400 103


unsigned char snippy ;


int main(){
  DDRB = 0xFF; 
  while (1)
  {
    
     USART_init(103);

     USART_transmit( "A");
     _delay_ms(200);
     USART_transmit( "B");
     _delay_ms(200);

  }

 return 1;
}

//*************************************************************
//USART_init: initializes the USART system
//*************************************************************

//page 133 of datasheet
void USART_init( unsigned int ubrr ) {

	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;

	UCSRA = 0x00; 
	UCSRB = 0x08;
	UCSRC = 0x86;

        //FROM 2313 DATASHEET 
        /* Enable receiver and transmitter */
        //UCSRB = (1<<RXEN)|(1<<TXEN);
	/* Set frame format: 8data, 2stop bit */
	//UCSRC = (1<<USBS)|(3<<UCSZ0);

        //MY BEST GUESS
        //control status reg A
        //UCSRA = 0x00; 
        //UCSRB = (1<<TXEN); //0x08;
        //UCSRC = (1<<USBS) |(1<<UCSZ1) | 1<<(UCSZ0);

}

void USART_transmit(unsigned char *data)
{
	while (!(UCSRA & (1 <<UDRE))); //wait for UDRE flag
	{
	;
	}
	UDR = *data;

}


void USART_tx_string( char *data )
{
while ((*data != '\0'))
   {
      while (!(UCSRA & (1 <<UDRE)));
      UDR = *data;
      data++;
   }   
}





