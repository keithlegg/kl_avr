/* -----------------------------------------------------------------------
 * Title: Change duty cycle PWM
 * Hardware: AT164pv 
 * (taken from 2313 demo) 
*THIS IS SET FOR- SERVO SPEED (18 MS duty, 1-2 MS ON) 
 -----------------------------------------------------------------------*/
 
 #define F_CPU 9600000UL // Define software reference clock for delay duration
 #include <avr/io.h>
 #include <util/delay.h>
 #include <avr/interrupt.h>
 #define LED PB3

 #define F_CPU 16000000UL

 int i ;


 int main (void)
 {

  DDRB |= (1 << LED); // OC0A on PB0

  ///////////////////////////
  // COM0A1 - COM0A0 (Set OC0A on Compare Match clear OC0A at TOP)
  // datasheet (TCCR0A ) page 104 

  TCCR0A |= (0 << COM0A0) | (1 << COM0A1)  | (1 << WGM01) | (1 << WGM00); 
  // initialize Output Compare Register A to 0
  OCR0A = 1; 

  //Clock Select bits datasheet page 77 //PRESCALER 
  TCCR0B |= (1<< CS02)|(1<<CS00)  ;


  for (;;)
  {
 
      for (i = 0 ; i < 50 ; i=i+10 ) // For loop (Up counter 0 - 255)
      {
	        OCR0A = i; // Update Output Compare Register (PWM 0 - 255)
	        _delay_ms(225);
      }
  
      for (i = 50 ; i > 0 ; i=i-10 ) // For loop (down counter 255 - 0 )
      {
	        OCR0A = i; // Update Output Compare Register (PWM 0 - 255)
	        _delay_ms(225);
      }
  }


  /////////////////////////// 

  }
