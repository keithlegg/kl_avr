/*

WORKS ON PIN CHANGE!

https://sites.google.com/site/qeewiki/books/avr-guide/external-interrupts

*/


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/delay.h>

#define LED PB0



int main(void)
{

    DDRB |= (1<<LED); // Set LED as output

   /////

    DDRD &= ~(1 << DDD2);     // Clear the PD2 pin
    // PD2 (INT0 pin) is now an input

    PORTD |= (1 << PORTD2);    // turn On the Pull-up
    // PD0 is now an input with pull-up enabled

    //took a wild guess here - kl
    EIMSK  = (1<<INT0);
    EICRA  = (1<<ISC00 ); 

    sei();                    // turn on interrupts
 

    while(1)
    {
        PORTB ^= (1 << 0);
        _delay_ms(500); 
    }
}



ISR (INT0_vect)
{
   PORTB |=(1<<0);
   _delay_ms(1000);
}



