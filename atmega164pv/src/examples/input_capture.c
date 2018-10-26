/*

DOES NOT WORK - YET

ISR's are triggering but not how I want 

*/
 


#define F_CPU 16000000UL
 

#include <avr/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

 
volatile uint16_t revTick;      // Ticks per revolution
volatile uint16_t revCtr;       // Total elapsed revolutions
 
void setupTimer()
{           
   TCCR1A = 0;      // normal mode
   TCCR1B = 68;     // (01000100) Rising edge trigger, Timer = CPU Clock/256 -or- 14745600/256 -or- 57.6KHz
   TCCR1C = 0;      // normal mode
   TIMSK1 = 33;     // (00100001) Input capture and overflow interupts enabled
   TCNT1  = 0;      // start from 0

}
 
ISR(TIMER1_CAPT_vect)  // PULSE DETECTED!  (interrupt automatically triggered, not called by main program)
{
   revTick = ICR1;      // save duration of last revolution
   TCNT1 = 0;           // restart timer for next revolution
   revCtr++;            // add to revolution count

   //PORTC |= (1<<PC0); //test of ICP pin 
}
 
ISR(TIMER1_OVF_vect)    // counter overflow/timeout
{ 
   revTick = 0; 
   //PORTC |= (1<<PC0); //test of overflow
}     
 
int main (void)
{
   sei();                // enable global interrupts.
   setupTimer();         // set timer perameters

   PORTD = (1<<6);       //pullup enabled
   DDRD  = (1<<6);       //ICR1 as input
   DDRC  = (1<<0);       // PC0 as output
 
   uint16_t RPM;       // Revolutions per minute


   while (1)
   {  // do calculations and talk to lcd while we're not doing anything...
 
      if (revTick > 0)                  //  DO NOT DIVIDE BY ZERO.
         {RPM = 3456000 / revTick;}     // 3456000 ticks/minute 
      else
         {RPM = 0;}
     /*
      PORTC |= (1<<PC0);    // test signal  120RPM
      _delay_ms( revTick );        
      PORTC  = (0<<PC0);
      _delay_ms( revTick ); 
     */

   }
 
   return 0;
}

 
