#define F_CPU 16000000L // Define software reference clock for delay duration
#include <avr/io.h>
#include <util/delay.h>



#define sbi(a, b) (a) |= (1 << (b))
#define cbi(a, b) (a) &= ~(1 << (b))


/*******************************/

/*
   DDRB |= (1 << 5);  //PB5 arduino led


   sbi(DDRB, 3 );     //PB5 arduino pin 11
   DDRB =0b00001000;  //PB3 arduino pin 11 
   DDRB |= (1 << 3);  //PB3 arduino pin 11


*/   


/*******************************/


int main (void)
{
   
    //DDRB =0b00001000;  //arduino pin 11 
    //DDRB |= (1 << 3);  //arduino pin 11

    //DDRD = 0xff;        
    //DDRD |= (0x4);  
    
    uint8_t dv = 100;

    DDRB = 0xff;

    while (1)
    {
        //PORTB ^= 0xff;
        //PORTD ^=  0xff; 

        sbi(PORTB,5 );
        _delay_ms(dv);

        cbi(PORTB,5 );
        _delay_ms(dv);

    }
} 

/*******************************/



