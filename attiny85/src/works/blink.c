#include <avr/io.h>
#include <util/delay.h>


int del =0;

int main (void)
{
  /* set PORTB for output*/
  DDRB = 0x01; 


  del =1000;

  while (1)
    {
      PORTB = 0x00;
      _delay_ms(del);
      PORTB = 0xff;
      _delay_ms(del);
  
    }

  return 1;
}
