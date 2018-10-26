/* -----------------------------------------------------------------------
 * Title: Change duty cycle PWM
 * Hardware: ATtiny2313
 * HACKED by keith
 -----------------------------------------------------------------------*/
 
 #define F_CPU 9600000UL // Define software reference clock for delay duration
 #include <avr/io.h>
 #include <util/delay.h>
 #include <avr/interrupt.h>
 #define LED PB0
 #define FOOBAR 0xff

 #define F_CPU 16000000UL

 int i ;
 
 int main (void)
 {
  //DDRB |= (1 << LED); // OC0A on PB0
  DDRB = FOOBAR;
 
  //PRESCALER AFFECTS TIMER VIA  TCCR0B

  cli(); //disable interrupts first
  CLKPR = (1 << CLKPCE);//enable
  CLKPR = (1 << CLKPS1 | 1 << CLKPS0);
  sei();//enable interrupts

  ///////////////////////////
  // COM0A1 - COM0A0 (Set OC0A on Compare Match, clear OC0A at TOP)

  //(1 << COM0A1) | (1 << COM0A0) = INVERT CONTROL 
  TCCR0A |= (1 << COM0A1) | (1 << COM0A0) | (1 << WGM01) | (1 << WGM00); 
  OCR0A = 1; // initialize Output Compare Register A to 0
 
  //timer/counter control sets the way prescaler affects timer   
  TCCR0B |= ( 1<< CS00);
 
  for (;;)
  {
 
      for (i = 0 ; i < 255 ; i++ ) // For loop (Up counter 0 - 255)
      {
	        OCR0A = i; // Update Output Compare Register (PWM 0 - 255)
	        _delay_ms(30);
      }
  
      for (i = 255 ; i > 0 ; i-- ) // For loop (down counter 255 - 0 )
      {
	        OCR0A = i; // Update Output Compare Register (PWM 0 - 255)
        	_delay_ms(30);
      }
  }
  /////////////////////////// 

  }
