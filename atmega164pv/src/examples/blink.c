#include <avr/io.h>
#include <util/delay.h>



int main (void)
{
  /* set PORTB for output*/
  DDRB = 0xFF;
  int del = 600;
  
  while (1)
    {
      PORTB = 0x01;
      _delay_ms(del);
      PORTB = 0x00;
      _delay_ms(del);
	  
	  //////
      PORTB = 0x02;
      _delay_ms(del);
      PORTB = 0x00;
      _delay_ms(del);
	  //////
      PORTB = 0x04;
      _delay_ms(del);
      PORTB = 0x00;
      _delay_ms(del);
	  //////
      PORTB = 0x02;
      _delay_ms(del);
      PORTB = 0x00;
      _delay_ms(del);	  
	  
      PORTB = 0x01;
      _delay_ms(del);
      PORTB = 0x00;
      _delay_ms(del);
	  

	  
	  
    }

  return 1;
}
