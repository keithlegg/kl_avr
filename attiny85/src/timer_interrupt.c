#include <avr/io.h>


/*
   timer/interrupts example for attiny85 

   pinchange , timer overflow and output compare 

   http://eleccelerator.com/avr-timer-calculator/

   Interrupts listed on page 48 of tiny85 datasheet 

*/


#define F_CPU   16000000UL
#define FOSC    16000000UL

#include <util/delay.h>
#include <avr/interrupt.h>

volatile long count        = 0; //count the pulses - measure timer on second pulse
volatile int is_triggered  = 0; //flag if timer excedes duration 

int main(void){
  DDRB  = 0x02; //LED OUT ON PIN 2
  PORTB = 0x00; //SET DEFUALT TO OFF
  sei();
  ////////////////////////

  //OUTPUT COMPARE - datasheet PAGE 69 
  //OCR1A = 50; //compare value 
 
  //timer control AB - dataheet page 77-79

  //CLEAR COMPARE - COMPARE INTERRUPT 
  //TCCR0A |= (1 << WGM01 ) ;   // CTC mode
  //TCCR0B |= (1 << CS00 );     // | (1 << CS02 );  
  TIMSK |= (1 << OCIE1A ) ;   // enable timer compare interrupt
  
  //TIMSK |= ( (1 << TOIE1));   //enable overflow interrupt 

  //turn on timer by setting prescaler (page 101) 
  TCCR1 |= (1 << CS10 ) |(1 << CS11 ) |(1 << CS12 ) |(1 << CS12 )   ; // Start timer 1 at Fcpu /64

  ////////////////////////
  //THESE ARE FOR PIN CHANGE INTERRUPT
  //GIMSK |=  (1<<PCIE);      // Enable pin change interrupt 
  //PCMSK =  0x01;            // Enable pin change interrupt for PB0 (Pin 5)
  //page 51 -PCIE bit is set (one) and the I-bit in the Status Register (SREG) is set (one),
  //SREG |= ( 1 << SREG_I );  // Enable global interrupts - WHY NOT SEI() ? 

  while(1){ 
      PORTB = 0x00;    

      //you can query or edit TCNT1 (timer counter) 
      //if(TCNT1==250) {



  }

}
///////////////////
//output compare interrupt

ISR ( TIMER1_COMPA_vect )
{
    PORTB = 0x02;
    //_delay_ms(1); 
}



