
#include <avr/io.h>

/*

1Mhz  /20000 = 20 ms pulses

*/

int main (void)
{

   DDRB |= 0x08; //11 is PB3 or 0x08, 13 is PB5 or 0x101  

   TCCR1B |= (1 << CS10); // Set up timer
   uint16_t c=0;

   //for (;;)
   while(1)
   {
      // Check timer value in if statement, true when count matches 1/20 of a second
      if (TCNT1 >= 20000)
      {
         if(c>1000){   
           PORTB ^= 0x08;//; // Toggle the LED
           c=0;
         }
         c++; 
         TCNT1 = 0; // Reset timer value
      }
   }

} 






