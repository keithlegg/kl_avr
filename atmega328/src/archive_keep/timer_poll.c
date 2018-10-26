#include <avr/io.h>

/*

1Mhz  /20000 = 20 ms pulses

*/



int main (void)
{
   DDRD |= 0xff;//(1 << 4); // Set LED as output

   //TCCR1B |= (1 << CS10); // Set up timer

    while(1){ PORTD ^= 0xff; }


   //for (;;)
   while(1)
   {
      // Check timer value in if statement, true when count matches 1/20 of a second
      if (TCNT1 >= 20000)
      {
         PORTB ^= 0x05; // Toggle the LED

         TCNT1 = 0; // Reset timer value
      }
   }

} 






