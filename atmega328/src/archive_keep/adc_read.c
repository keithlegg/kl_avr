/*VARIOUS ADC CODES

  I wasted the afternoon of Dec 4, 2013 by passing the value to delay - the fix was in "mydelay"

*/



#define F_CPU	                        16000000L
//#define __DELAY_BACKWARD_COMPATIBLE__ 
#include <util/delay.h>

#include <avr/io.h>
#include <stdint.h>       // needed for uint8_t






int ADCsingleREAD(uint8_t adctouse)
{
    uint16_t ADCval;
    //DDRC &=~(1<<adctouse); //?

    ADMUX  =      adctouse;    // use #1 ADC - page 261 on datasheet
    ADMUX  |= (1 << REFS0);    // use AVCC as the reference
    ADMUX  &= ~(1 << ADLAR);   // clear for 10 bit resolution
    
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);    // 128 prescale for 8Mhz (SLOW!) 
    ADCSRA |= (1 << ADPS1);    

    ADCSRA |= (1 << ADEN);                                  // Enable the ADC
    ADCSRA |= (1 << ADSC);                                  // Start the ADC conversion
    while(ADCSRA & (1 << ADSC));                            // waits for the ADC to finish 

    ADCval = ADCL;
    ADCval = (ADCH << 8) + ADCval; // (| or + ?) -  ADCH is read so ADC can be updated again
    return ADCval;
 
}






void mydelay(uint16_t count) {
  while(count--) {
    _delay_ms(1);

  }
} 

/////////

int main(void)
{
    uint16_t ADCvalue =0;
    
    //ADCInit(0);


    /* set PORTB for output*/
    DDRB = 0x08;

    while (1)
    {
        ADCvalue = ADCsingleREAD(0);
        mydelay(ADCvalue); 
        PORTB ^= 0x08;

    }
}




