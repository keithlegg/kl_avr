/*
 ATMEGA328p
 // -U lfuse:w:0xff:m -U hfuse:w:0xd9:m 

*/


#define F_CPU 16000000
#define BAUD 9600

#include <avr/io.h>
#include <util/delay.h>
#include <util/setbaud.h>

void inituart(void)
{
   UBRR0H = UBRRH_VALUE;
   UBRR0L = UBRRL_VALUE;

   //found this , seems relavant 
   UCSR0A |= (1 << U2X0);

   UCSR0B = (1 << RXEN0)  | (1 << TXEN0);    // Enable UART receiver and transmitter
   //UCSR0C = (1 << UCSZ02) | (1 << UCSZ01);   // set to 8 data bits, 1 stop bit
   UCSR0C = (1<<USBS0)|(3<<UCSZ00);  // set to 8 data bits, 2 stop bits

}


void transmitbyte (unsigned char data)
{   
   while (!(UCSR0A & (1 << UDRE0)));   // Wait for empty transmit buffer
   UDR0 = data;                       // Start transmittion
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




