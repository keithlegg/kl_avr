/*
 ATTINY2313 
 HFUSE=D4
 LFUSE=E4
 EFUSE=FF
*/

// 9600 19200 38400 57600 115200


#define F_CPU 16000000UL
#define BAUD 9600

#include <avr/io.h>
#include <util/delay.h>
#include <util/setbaud.h>

void inituart(void)
{
   UBRRH = UBRRH_VALUE;
   UBRRL = UBRRL_VALUE;

   //found this , seems relavant 
   UCSRA |= (1 << U2X);


   UCSRB = (1 << RXEN)  | (1 << TXEN);    // Enable UART receiver and transmitter
   UCSRC = (1 << UCSZ1) | (1 << UCSZ0);   // set to 8 data bits, 1 stop bit
}


void transmitbyte (unsigned char data)
{   
   while (!(UCSRA & (1 << UDRE)));   // Wait for empty transmit buffer
   UDR = data;                       // Start transmittion
}


int main(void) {
   DDRB |= 1<<PB0;   /* set PB0 to output */
   inituart();
    while(1) {
      _delay_ms(100);
        transmitbyte('A');

      //PORTB ^= 1<<PB0;
    }
    return 0;
} 




