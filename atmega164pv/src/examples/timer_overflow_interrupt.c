

// this code sets up a timer0 for 4ms @ 16Mhz clock cycle
// an interrupt is triggered each time the interval occurs.



#include <avr/io.h> 
#include <avr/interrupt.h>
#include <avr/delay.h> 

int main(void)
{
    DDRB |= (1 << 0); // Set LED as output

    //data sheet page 94  
    // Set the Timer Mode to CTC
    TCCR0A |= (1 << WGM01);

    // Set the value that you want to count to
    OCR0A = 0xF9;

    TIMSK0 |= (1 << OCIE0A);    //Set the ISR COMPA vect

    sei();         //enable interrupts



    TCCR0B |= (1 << CS02);
    // set prescaler to 256 and start the timer


    while (1)
    {
        //main loop
    }
}


ISR (TIMER0_COMPA_vect)  // timer0 overflow interrupt
{
      PORTB ^= (1 << 0); // Toggle the LED
      _delay_ms(500); 
}


/*
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
   INPUT(D,6);              // Input Capture Pin set as input
   SET(D,6);                // Enable pull-up resistor
   OUTPUT(B,0);             // Declare PB0 as output
   TCCR1B = (1<<CS11);      // Set prescaler to 8
   TCCR1B |= (1<<ICES1);    // Input capture in rising edge
   TIMSK0 = (1<<ICIE1);     // Enable Input Capture interrupt

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

*/

