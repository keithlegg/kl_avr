// 9.6 MHz, built in resonator
#define F_CPU 9600000
#define LED PB1
 
#include <avr/io.h>


//--------------------------//

adc_setup:
    // Set the ADC input to PB2/ADC1
    //ADMUX |= (1 << MUX0);
    //ADMUX |= (1 << ADLAR);
 
    // Set the prescaler to clock/128 & enable ADC
    // At 9.6 MHz this is 75 kHz.
    // See ATtiny13 datasheet, Table 14.4.
    //ADCSRA |= (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);

 
//--------------------------//

adc_read:
    // Start the conversion
    //ADCSRA |= (1 << ADSC);
 
    // Wait for it to finish
    //while (ADCSRA & (1 << ADSC));
    ret //ADCH;

//--------------------------// 
pwm_setup:
    // Set Timer 0 prescaler to clock/8.
    // At 9.6 MHz this is 1.2 MHz.
    // See ATtiny13 datasheet, Table 11.9.
    //TCCR0B |= (1 << CS01);
 
    // Set to 'Fast PWM' mode
    //TCCR0A |= (1 << WGM01) | (1 << WGM00);
 
    // Clear OC0B output on compare match, upwards counting.
    //TCCR0A |= (1 << COM0B1);

//--------------------------//
pwm_write:
    //OCR0B = val;

//--------------------------// 
main:
    // LED is an output.
    //DDRB |= (1 << LED);  
 
    //adc_setup();
    //pwm_setup();
  
    //while (1) {
    //    adc_in = -adc_read(); //negate the value !
    //    pwm_write(adc_in-1);
    
