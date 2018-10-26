
// this code sets up a timer0 for 1ms @ 16Mhz clock cycle
// in order to function as a time delay at the begining of the main loop
// NOTE:    1ms = 0xF9 but we are going 0xFF, so we are actually creating a
//          1.024ms timer.

//https://sites.google.com/site/qeewiki/books/avr-guide/timer-on-the-atmega8


#include <avr/io.h>


int main(void)
{

    while (1)
    {

        // start the timer
        TCCR0 |= (1 << CS01) | (1 << CS00);
        // set prescaler to 64 and start the timer

        while ( (TIFR & (1 << TOV0) ) > 0)        // wait for the overflow event
        {
        }
       
        TIFR &= ~(1 << TOV0);
        // reset the overflow flag

    }
}
