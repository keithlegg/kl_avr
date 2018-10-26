


#include <avr/io.h>
#include <avr/interrupt.h>

volatile uint16_t tick; 

int main(void)
{

    DDRB = 0x08;//output LED 


    DDRD &= ~(1 << DDD2);     // Clear the PD2 pin
    PORTD |= (1 << PORTD2);    // turn On the Pull-up

    EICRA |= (1 << ISC00);    // set INT0 to trigger on ANY logic change
    EIMSK |= (1 << INT0);     // Turns on INT0
    sei();                    // turn on interrupts

    //timer stuff
    TCCR1B = 1; //start Timer1


    /////
    while(1)
    {
    }
}



ISR (INT0_vect)
{
   //PORTB ^= 0x08;  //TO TEST 
   tick=TCNT1;
   //WE HAVE A CAPTURE VALUE! = tick
   if (tick<250)
   {
      PORTB = 0x08;
   }else{
      PORTB = 0x00;
   }


   TCNT1=0;
}



