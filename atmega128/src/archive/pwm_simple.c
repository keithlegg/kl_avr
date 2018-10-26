#include <avr/io.h>

#define F_CPU 16000000UL //AVR Clock Speed in MHZ
#define FOSC 16000000    // Clock Speed

#include <util/delay.h>


void pwm_setup (void)
{
    // See datasheet, Table 56.
    TCCR0 |= (1 << CS00);
 
    // Set to 'Fast PWM' mode
    TCCR0 |= (1 << WGM01) | (1 << WGM00);
 
    // Clear OCR0 output on compare match, upwards counting.
    TCCR0 |= (1 << COM01) | (1 << COM00);
}
 
void pwm_write (uint16_t val)
{
    OCR0 = val;
}
 


/////////////////

int main (void)
{
   //DDRB =0b00001000;   
   //DDRB |= (1 << 3);   
   DDRB = 0xff;        

   pwm_setup();


   while (1)
   {

       uint16_t c = 0;
       for(c=0;c<512;c++){
           pwm_write(c); 
       }
       
       //pwm_write(512);
       //PORTB = PORTB ^ 0xff;
   }
} 

/////////////////



