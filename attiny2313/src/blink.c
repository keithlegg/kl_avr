#include <avr/io.h>
#include <util/delay.h>

int main (void)
{
   DDRB |= (1 << 0); // Set LED as output


   while (1)
   {

	      PORTB ^= 0x01;
	      _delay_ms(7000);


   }
} 
