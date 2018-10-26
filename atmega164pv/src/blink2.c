#include <avr/io.h>

#define F_CPU   16000000UL


#include <avr/delay.h>





int main (void)
{
   DDRB |= (1 << 0); // Set LED as output
   
   for (;;)
   {
        PORTB ^= (1 << 0); // Toggle the LED
         _delay_ms(100);   //this is HALF of what I expect (high and low for square wave)

   }
} 
