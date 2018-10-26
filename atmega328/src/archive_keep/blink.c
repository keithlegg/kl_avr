#define F_CPU 16000000L // Define software reference clock for delay duration
#include <avr/io.h>
#include <util/delay.h>


/////////////////

int main (void)
{
   //DDRB =0b00001000;  //arduino pin 11 
   //DDRB |= (1 << 3);  //arduino pin 11
   DDRD = 0xff;       //arduino pin 11

   //DDRD |= (0x4); // PORTD!


   while (1)
   {
       //PORTB = 0xff;
       PORTD ^=  0xff; 

       _delay_ms(100);

   }
} 

/////////////////



