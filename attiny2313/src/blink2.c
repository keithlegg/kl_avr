#include <avr/io.h>

int main (void)
{
   DDRB |= (1 << 0); // Set LED as output

   for (;;)
   {
       PORTB ^= (1 << 0); // Toggle the LED

   }
} 
