#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//#define F_CPU 16000000UL



int main (void)
{

  DDRB = (1 <<PORTB0 | 1<<PORTB1); //portb bits 0-1 output  //DDRB = 0x03;


  int del = 100;
  
  
  cli(); //disable interrupts first
  CLKPR = (1 << CLKPCE);//enable
  CLKPR = (1 << CLKPS1 | 1 << CLKPS0);
  sei();//enable interrupts

  while(1){
      //_delay_ms(del);
      PORTB = 0x01;
      //_delay_ms(del);     
      PORTB = 0x00;
      //_delay_ms(del);

  }

  return 1;
}
