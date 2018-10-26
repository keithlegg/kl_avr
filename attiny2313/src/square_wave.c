#include <avr/io.h>
#include <util/delay.h>

int main (void)
{
   DDRB |= (1 << 0); // Set LED as output

   // TODO: Set up timer

   while (1)
   {
      // TODO: Check timer value in if statement, true when count matches 1/20 of a second

      PORTB ^= (1 << 0); // Toggle the LED
      //PORTB = 0x01;
      //PORTB = 0x00;

         // TODO: Reset timer value

   }
} 
