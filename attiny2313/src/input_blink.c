#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//#define F_CPU 16000000UL

//#include <avr/interrupts.h>



int main (void)
{

  DDRB = (1 <<PORTB0 | 1<<PORTB1); //portb bits 0-1 output  //DDRB = 0x03;

  //DDRA = 0x00; //port A input 
  //PORTA = 0X00; // set internal pullups ?

  int del = 100;
  
  
  cli(); //disable interrupts first
  CLKPR = (1 << CLKPCE);//enable
  CLKPR = (1 << CLKPS1);
  sei();//enable interrupts

  while(1){
      //_delay_ms(del);
      PORTB = 0x01;
      //_delay_ms(del);     
      PORTB = 0x00;
      //_delay_ms(del);

  }

  
  /*
  while (1)
    {
      //if a1 is on , b 0,1 blinks
      if(PINA==0x01)      
      {
	      _delay_ms(del);
	      PORTB = 0x01;
	      _delay_ms(del);     
	      PORTB = 0x00;
	      _delay_ms(del);

	      PORTB = 0x02;  
	      _delay_ms(del);     
	      PORTB = 0x00;
	      _delay_ms(del);	  
      }

      //if A is off B is off
      if(PINA==0x00)      
      {
            PORTB=0X00;   
      }	

    }
  */

  return 1;
}
