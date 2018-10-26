/*

DOES NOT WORK - YET 

untested - from 

https://sites.google.com/site/qeewiki/books/avr-guide/analog-input


*/


// this code scans ADC1 for an analog signal upon request, using 8Mhz processor clock


#include <avr/io.h>
//#include <stdint.h>       // needed for uint8_t
//#include <avr/delay.h>


int ADCsingleREAD(uint8_t adctouse)
{
    int ADCval;

    ADMUX = adctouse;         // use #1 ADC
    //ADMUX |= (1 << REFS0);    // use AVcc as the reference
    ADMUX &= ~(1 << ADLAR);   // clear for 10 bit resolution
    
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);    // 128 prescale for 8Mhz
    ADCSRA |= (1 << ADEN);    // Enable the ADC

    ADCSRA |= (1 << ADSC);    // Start the ADC conversion

    while(ADCSRA & (1 << ADSC));      // Thanks T, this line waits for the ADC to finish 


    ADCval = ADCL;
    ADCval = (ADCH << 8) + ADCval;    // ADCH is read so ADC can be updated again

    return ADCval;
}

    

int main(void)
{
    int ADCvalue;

    DDRB |= (1 << 0); // Set LED as output

    while (1)
    {
            ADCvalue = ADCsingleREAD(0);

            //ADCvalue = 103;
            // ADCvalue now contains an 10bit ADC read
            PORTB = (1 << 1);
            //_delay_ms(ADCvalue);   //this is HALF of what I expect (high and low for square wave)
            PORTB = (1 << 0);
            //_delay_ms(ADCvalue);
    }
}













