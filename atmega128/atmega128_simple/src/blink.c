#include <avr/io.h>

#define F_CPU 16000000UL //AVR Clock Speed in MHZ
#define FOSC 16000000    // Clock Speed

#include <util/delay.h>

/*
  PORT MAPPING FOR KEITHS BOARD 
  ----------------
  PORTF--  --PORTA
  PORTE--  --PORTC
  ----------------
  PORTB--  --JUNK
  PORTD--  --JUNK
  ----------------
*/

int main (void)
{
   DDRA = 0xff; //binary 11111111

   while (1)
   {
       PORTA ^= 0x08;  //XOR (flash) pin 4 of portF
       _delay_ms(350); //wait 350 miliseconds, notice we are including <util/delay.h> above to get this

   }
} 





