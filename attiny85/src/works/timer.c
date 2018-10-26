#include <avr/io.h>

/*

Basic timer example for attiny85 

*/


#define F_CPU   16000000UL
#define FOSC    16000000UL

#include <util/delay.h>

////////////////////

void blink(){
    PORTB ^= 0xff;
    _delay_ms(100);

}

////////////////////

int main(void){
 DDRB = 0xff; 
 PORTB = 0x00;
 int a = 0;
 
 //turn on timer by setting precaler
 TCCR1 |=  1<<CS10; //PRECALER/ page 89 

  while(1){ 

    //TCNT1 is an 8 bit register!
    if (TCNT1>250){
       TCNT1=0; 
       PORTB ^= 0xff;
       _delay_ms(500);
    }
    


  }

}

////////////////////



