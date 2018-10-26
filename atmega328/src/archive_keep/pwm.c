/* -----------------------------------------------------------------------
* Title: Change duty cycle PWM
* Hardware: ATtiny13
-----------------------------------------------------------------------*/
 
#define F_CPU 16000000L // Define software reference clock for delay duration
#include <avr/io.h>
#include <util/delay.h>

/*
20 ms = 50hz
16000000/50 = 320000
320000/256 = 1250


*/


//https://sites.google.com/site/qeewiki/books/avr-guide/pwm-on-the-atmega328
// this code sets up counter0 for an 8kHz Fast PWM wave @ 16Mhz Clock
int main(void)
{
    DDRD |= (1 << DDD6);
    // PD6 is now an output

    OCR0A = 10;
    // set PWM for 50% duty cycle


    TCCR0A |= (1 << COM0A1);
    // set none-inverting mode

    TCCR0A |= (1 << WGM01) | (1 << WGM00);
    // set fast PWM Mode

    TCCR0B |= (1 << CS01);
    // set prescaler to 8 and starts PWM


    while (1);
    {
        // we have a working Fast PWM
    }
}

