

// 9.6 MHz, built in resonator
#define F_CPU 9600000

//#define errorled PB0
//#define coildriver PB1

 
#include <avr/io.h>
#include <util/delay.h>


#define del 10

int main (void)
{
  /* set PORTB for output*/
  DDRB = 0xff; 

  while (1)
    {
      PORTB = 0x01;
      _delay_ms(del);
      
      PORTB = 0x02;
      _delay_ms(del);

      PORTB = 0x04;
      _delay_ms(del);
  
    }

  return 1;
}


//uint8_t halted = 0;
//uint16_t err_cnt = 0;



/*
void adc_setup (void)
{
    // Set the ADC input to PB2/ADC1
    ADMUX |= (1 << MUX0);
    ADMUX |= (1 << ADLAR);
 
    // Set the prescaler to clock/128 & enable ADC
    ADCSRA |= (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);
}
 
int adc_read (void)
{
    // Start the conversion
    ADCSRA |= (1 << ADSC);
 
    // Wait for it to finish
    while (ADCSRA & (1 << ADSC));
 
    return ADCH;
}
 
void pwm_setup (void)
{
    // Set Timer 0 prescaler to clock/8.
    // At 9.6 MHz this is 1.2 MHz.
    // See ATtiny13 datasheet, Table 11.9.
    TCCR0B |= (1 << CS01);
 
    // Set to 'Fast PWM' mode
    TCCR0A |= (1 << WGM01) | (1 << WGM00);
 
    // Clear OC0B output on compare match, upwards counting.
    TCCR0A |= (1 << COM0B1);
}
 
void pwm_write (int val)
{
    OCR0B = val;
}
 
int main (void)
{
    int adc_in;
    int value = 0;

    DDRB = 0x03;//lower two bits
 
    adc_setup();
    pwm_setup();
  
    //PORTB |= (0x01); //turn on error LED

    while (1) {
        if (!halted){
            adc_in = -adc_read();  //negate the value !
            pwm_write(adc_in);
        }

        //problems - this WOULD NOT work if I used a variable
        //if we read twice it runs HALF AS FAST!!! GRR
        if(adc_read()<200){ 
            //TCCR0A &=~COM0B1;// disable pwm output pin
            PORTB |=  (0x01); //turn on error LED
            //halted=1;
        }else{
            // // TCCR0A |= COM0B1; //ENABLE PWM 
            PORTB &= ~(0x01); //turn off error LED
            //    PORTB ^= (0x01);
        }
            
    }
}

*/
