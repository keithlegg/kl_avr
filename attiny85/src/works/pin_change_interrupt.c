#include <avr/io.h>


/*
   Basic timer example for attiny85 

   a pin-change interrupt and the timer module. 

   When the pin first changes states (to indicate start-of-pulse), clear the timer, then when it changes again 
   read the timer to determine the pulse width.

   INT0    = External Interrupt Request 0
   PCINT0  = Pin Change Interrupt Request 0
*/


#define F_CPU   16000000UL
#define FOSC    16000000UL

#include <util/delay.h>
#include <avr/interrupt.h>


////////////////////

void blink(){
  int a= 0;
  for (a=0;a<20;a++){
    PORTB ^= 0x02;
    _delay_ms(100);
  }
}

////////////////////





int main(void){
  DDRB = 0x02; 
  PORTB = 0x00;
  sei();
 
  //INTERRUPT STUFF - turn on external interrupt 
  GIMSK |=  (1<<PCIE);      // Enable pin change interrupt 
  PCMSK =  0x01;            // Enable pin change interrupt for PB0 (Pin 5)
  //data sheet page 51 - When the PCIE bit is set (one) and the 
  //I-bit in the Status Register (SREG) is set (one), pin change interrupt is enabled
  SREG |= ( 1 << SREG_I );  // Enable global interrupts 


  //TIMER STUFF - turn on timer by setting prescaler
  //TCCR1 |=  1<<CS10; //PRECALER/ page 89 

  while(1){ 

    //blink();

  }

}



ISR( PCINT0_vect) //INT0_vect)  //extern interrupt0 service routine     
{
  blink();
} 


/*
  unsigned int tick;   
   
   tick=TCNT1;
   TCNT1=0;               //clear timer

   if (tick < 4000)       //channel pulse detected
   {
    puls_array[ind]=tick;  //save value of channel 'ind'
    ind++;                 //next channel
   }
   
   else                   //synchro gap detected
   {
    TCNT1=0;
    ind=0;
   }
*/


////////////////////
/*
 //EXAMPLE TO READ TIMER VALUE 

    //TCNT1 is an 8 bit register!
    if (TCNT1>250){
       TCNT1=0; 
       PORTB ^= 0xff;
       _delay_ms(500);
    }
*/


////////////////////


////////////////////


