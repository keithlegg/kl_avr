#include <avr/io.h>
#include <avr/interrupt.h>

#define READ(port,pin) (PIN ## port & (1<<pin))
#define INPUT(port,pin) DDR ## port &= ~(1<<pin)
#define OUTPUT(port,pin) DDR ## port |= (1<<pin)
#define CLEAR(port,pin) PORT ## port &= ~(1<<pin)
#define SET(port,pin) PORT ## port |= (1<<pin)
#define TOGGLE(port,pin) PORT ## port ^= (1<<pin)

//https://forum.sparkfun.com/viewtopic.php?t=19993


volatile unsigned int pWidth;
volatile unsigned int pStart;
volatile unsigned int pStop;


int main (void)
{
   INPUT(D,6);   // Input Capture Pin set as input
   SET(D,6);     // Enable pull-up resistor
   OUTPUT(B,0);            // Declare PB0 as output
   TCCR1B = (1<<CS11);     // Set prescaler to 8
   TCCR1B |= (1<<ICES1);   // Input capture in rising edge
   TIMSK = (1<<ICIE1);     // Enable Input Capture interrupt

   pWidth = 1500;

   sei();//sure why not (see below)

   while(1)
   {
      //sei(); //should be outside of loop?
      if(pWidth <= 1470)
      {
         PORTB |= (1<<PB0);
      }
      else
      {
         PORTB &= ~(1<<PB0);
      }
   }
}

ISR(TIMER1_CAPT_vect)
{

   PORTB ^= (1<<PB0);
    pStop = ICR1;
    if (TCCR1B & (1<<ICES1))
   {
        TCCR1B &= ~(1<<ICES1);
        pStart = pStop;
    }
   else
   {
        TCCR1B |= (1<<ICES1);
        pWidth = pStop - pStart;
    }
}
