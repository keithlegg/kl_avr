#include <avr/io.h>
#include <avr/interrupt.h>


int main ( void )
{
  DDRB |= 0x08 ;        // Set LED as output
  TCCR1B |= (1 << WGM12 ) ; // Configure timer 1 for CTC mode
  TIMSK1 |= (1 << OCIE1A ) ; // Enable CTC interrupt
  
  
  sei(); //Enable global interrupts
  
  OCR1A = 65500; // Set CTC compare value to 1 Hz at 1 MHz AVR clock , with a prescaler of 64
  //OCCR'S - To do a 16-bit write, the high byte must be written before the low byte. 
  //For a 16-bit read, the low byte must be read before the high byte.
  
  TCCR1B |= ((1 << CS10 ) | (1 << CS11 ) ) ; // Start timer at Fcpu /64

  for (;;)
  {
  }
  
}

ISR ( TIMER1_COMPA_vect )
{
  PORTB ^= 0x08 ; // Toggle the LED
}


