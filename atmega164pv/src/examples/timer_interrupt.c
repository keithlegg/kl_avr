

// this code sets up timer2 for a 250us  @ 16Mhz Clock


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>



int main(void)
{

    DDRB |= (1 << 0); // Set LED as output

    OCR2A = 62;
    //OCR2A = 250;

    TCCR2A |= (1 << WGM21);
    // Set to CTC Mode

    TIMSK2 |= (1 << OCIE2A);
    //Set interrupt on compare match

    TCCR2B |= (1 << CS21);
    // set prescaler to 64 and starts PWM

    sei();
    // enable interrupts


    while (1);
    {
      //PORTB = (1 << 0); // Toggle the LED
    }
}

ISR (TIMER2_COMPA_vect)
{
      PORTB ^= (1 << 0); // Toggle the LED
      //_delay_ms(100); 
}





/*

//WORKS GREAT !!! (kl) 

// this code sets up a timer0 for 4ms @ 16Mhz clock cycle
// an interrupt is triggered each time the interval occurs.


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
  //    _delay_ms(500); 
}

*/


