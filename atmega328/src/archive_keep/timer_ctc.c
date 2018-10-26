#include <avr/io.h>


int main ( void )
{
  DDRB |= (1 << 0) ; // Set LED as output
  TCCR1B |= (1 << WGM12 ) ; // Configure timer 1 for CTC mode
  OCR1A  = 15624; // Set CTC compare value to 1 Hz at 1 MHz AVR clock , with a prescaler of 64
  TCCR1B |= ((1 << CS10 ) | (1 << CS11 ) ) ; // Start timer at Fcpu /64
  for (;;)
  {
    if ( TIFR1 & (1 << OCF1A ) )
    {
      PORTB ^= (1 << 0) ; // Toggle the LED
      TIFR1 = (1 << OCF1A ) ; // clear the CTC flag ( writing a logic one to the set flag clears it )
    }
  }
}





