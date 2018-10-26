/*


DOES NOT WORK!

*/


#include <avr/io.h>

#include <avr/interrupt.h>


#define F_CPU	                        16000000UL
//#define FOSC	                        16000000L

#include <avr/delay.h>


 #define LED PB0

//////////////////////



/**
* Inits the master timer to interrupt every 20ms. (for an 8mhz chip)
*/
void timer_init(void)
{
    // Mode 2 (CTC, OCRA as top) and 1/1024 prescaler.
    TCCR2A |= (1<<WGM21);
    TCCR2B |= ((1<<CS20)|(1<<CS21)|(1<<CS22));   

    // Set top.
    OCR2A = 156;

    // Our event timer will be normal mode with div 1024.
    TCCR0B |= ((1<<CS00)|(1<<CS02));
    TIMSK0 |= (1<<TOIE0);
}


int main (void)
{

  // Enable global interrupts.
  //timer_init();
  //sei();

  //////////

   DDRB |= (1 << 0); // Set LED as output
   uint8_t count = 0;
   while(1){
	   for (count=0;count<20;count++)
	   {
		//PORTB ^= (1 << 0); // Toggle the LED
                PORTB =0x00;

                /*
		if (count<5){
		  PORTB =0x01;
		}else{
		  PORTB =0x00;
		}


		_delay_ms(2.0);   //this is HALF of what I expect?
               */
 
		//PORTB = 0x01;   
		//_delay_ms(1);
	       // PORTB = 0x00;   
		//_delay_ms(1);

	   }
    }
} 



 ISR (INT0_vect) // Interrupt on Int0 vector
 {
 
   PORTB &= ~(1 << LED); // Set 0 on LED pin (led turn on)
   _delay_ms (1000);
 }
 


