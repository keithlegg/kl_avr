#include <avr/io.h>


/*
   Basic timer example for attiny85 

   a pin-change interrupt and the timer module. 

   When the pin first changes states (to indicate start-of-pulse), clear the timer, then when it changes again 
   read the timer to determine the pulse width.

   INT0    = External Interrupt Request 0
   PCINT0  = Pin Change Interrupt Request 0


  The interrupts can detect four different types of pin changes:
  1. pin goes low
  2. any logical change in pin
  3. falling edge (pin goes from high to low)
  4. rising edge (ping goes from low to high)

*/


#define F_CPU   16000000UL
#define FOSC    16000000UL

#include <util/delay.h>
#include <avr/interrupt.h>

volatile int count        = 0; //count the pulses - measure timer on second pulse
volatile int is_triggered = 0; //flag if timer excedes duration 

int main(void){
  DDRB  = 0x02; //LED OUT ON PIN 2
  PORTB = 0x00; //SET DEFUALT TO OFF
  sei();

  //DATASHEET PAGE 69 
  //TCCR0A |= (1 << WGM01 ) ;                //CTC mode
  TCCR0B |= (1 << CS00 );// | (1 << CS02 );  //set prescaler

  //OCR1A = 200;
  TIMSK |= (1 << OCIE1A ) ; //mask register - enable timer compare interrupt
  TCCR1 |= (1 << CS10 ) |(1 << CS11 ) | (1 << CS12 ) ; // Start timer at Fcpu /64

  ////////////////////////

  //TIMER STUFF - turn on timer by setting prescaler
  TCCR1 |=  1<<CS10; //PRECALER/ page 89 

  ////////////////////////
  //THESE ARE FOR PIN CHANGE INTERRUPT
  //GIMSK |=  (1<<PCIE);      // Enable pin change interrupt 
  //PCMSK =  0x01;            // Enable pin change interrupt for PB0 (Pin 5)
  //page 51 -PCIE bit is set (one) and the I-bit in the Status Register (SREG) is set (one),
  //SREG |= ( 1 << SREG_I );  // Enable global interrupts 

  while(1){ 
    if (is_triggered==1){
       PORTB = 0x02;
    }else{
       PORTB = 0x00;    
    }

  }

}
///////////////////
//output compare interrupt

ISR ( TIMER1_COMPA_vect ) 
{
  PORTB = 0xff;
}


///////////////////
/*
//pin change interrupt

ISR( PCINT0_vect)     
{
   //second change
   if (count==1){
     if(TCNT1<20){ //sample time to get value - 8 bit timer 255 max
       is_triggered=1;
       TCNT1=0;
     }else{
       is_triggered=0;
     }
     count=0;
   }
   //first change
   if (count==0){
    TCNT1=0; //clear timer register to begin count
    count++; 
   }

} 
*/





