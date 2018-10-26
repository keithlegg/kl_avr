/* -----------------------------------------------------------------------
 * timer interrupt test - triggering but not working
 * Hardware: ATtiny85
 //this counts up and triggers and interrupt when Output Compare value is met 
 -----------------------------------------------------------------------*/
 
 

#define F_CPU   16000000UL
#define FOSC    16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
 
int i;

 
int main(void) {
 //DATASHEET PAGE 69 
 //TCCR0A |= (1 << WGM01 ) ;                //CTC mode
 TCCR0B |= (1 << CS00 );// | (1 << CS02 );  //set prescaler

 //OCR1A = 200;
 TIMSK |= (1 << OCIE1A ) ; //mask register - enable timer compare interrupt
 TCCR1 |= (1 << CS10 ) |(1 << CS11 ) | (1 << CS12 ) ; // Start timer at Fcpu /64

 sei(); // int - Global enable interrupts
 
 DDRB = 0xff; 
 
 for (;;) // loop (endless)
 {

   PORTB = 0x00;
 }
 
 return 0;
}
 

ISR ( TIMER1_COMPA_vect ) 
{
  PORTB = 0xff;
  //_delay_ms (1);
}

