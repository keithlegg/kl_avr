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

  /*
  cli(); //disable interrupts first
  CLKPR = (1 << CLKPCE);//enable
  CLKPR = (1 << CLKPS1 ); //| 1 << CLKPS0
  sei();//enable interrupts
  */

  ///////////////////////////
  // COM0A1 - COM0A0 (Set OC0A on Compare Match clear OC0A at TOP)
  //WGM datasheet (TCCR0A ) page 75 , page 94 , (TCCR1A) page 106 
  //INVERT 1 com0a0,1 com0a1 , NON INVERT 0 com0a0,1 com0a1
  TCCR0A |= (0 << COM0A0) | (1 << COM0A1)  | (1 << WGM01) | (1 << WGM00); 
  OCR0A = 1; // initialize Output Compare Register A to 0

  //Clock Select bits datasheet page 77 
  TCCR0B |= 1<< CS00  ;


  for (;;)
  {
 
      for (i = 0 ; i < 255 ; i=i+10 ) // For loop (Up counter 0 - 255)
      {
	        OCR0A = i; // Update Output Compare Register (PWM 0 - 255)
	        _delay_ms(5);
      }
  
      for (i = 255 ; i > 0 ; i=i-10 ) // For loop (down counter 255 - 0 )
      {
	        OCR0A = i; // Update Output Compare Register (PWM 0 - 255)
	        _delay_ms(5);
      }
  }


  /////////////////////////// 

  }
