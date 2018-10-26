/*
 ATTINY2313 
 HFUSE=D9
 LFUSE=FF
 EFUSE=FF


THIS WORKS AT 9600 , 16 MHZ ???

16000000 MHZ , 19200 BAUD , WORKS AT 9600 

 */

#define F_CPU 16000000UL
#define BAUD 9600

#include <avr/io.h>
#include <util/delay.h>
#include <util/setbaud.h>

void inituart(void)
{
   UBRRH = UBRRH_VALUE;
   UBRRL = UBRRL_VALUE;
   
   UCSRB = (1 << RXEN)  | (1 << TXEN);      // Enable UART receiver and transmitter
   UCSRC = (1 << UCSZ1) | (1 << UCSZ0);   // set to 8 data bits, 1 stop bit
}


void transmitbyte (unsigned char data)
{   
   while (!(UCSRA & (1 << UDRE)));   // Wait for empty transmit buffer
   UDR = data;                  // Start transmittion
}


int main(void) {
   DDRB |= 1<<PB0;   /* set PB0 to output */
   inituart();
    while(1) {
        transmitbyte('a');
        transmitbyte('b');
        transmitbyte('c');
        transmitbyte('d');

      _delay_ms(10);
      PORTB ^= 1<<PB0;
    }
    return 0;
} 
