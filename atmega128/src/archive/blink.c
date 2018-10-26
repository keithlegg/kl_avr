#include <avr/io.h>

#define F_CPU 16000000UL //AVR Clock Speed in MHZ
#define FOSC 16000000    // Clock Speed

#include <util/delay.h>

/////////////////

int main (void)
{
   //DDRB =0b00001000;  //arduino pin 11 
   //DDRB |= (1 << 3);  //arduino pin 11
   DDRF = 0x08;       //arduino pin 11

   //DDRD |= (0x4); // PORTD!


   while (1)
   {
       PORTF ^= 0x08;
       //PORTD ^=  0x4; //PORTD!

       //_delay_ms(500);

   }
} 

/////////////////



