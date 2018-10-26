/*VARIOUS ADC CODES

*/


#define F_CPU	                        16000000L
#include <util/delay.h>

#include <avr/io.h>
#include <stdint.h>       // needed for uint8_t

////////////

#define SERVO_MIN                       0
#define SERVO_NEUTRAL                   4000
#define SERVO_MAX                       8000

#define SERVO_MIN_LOW                   0
#define SERVO_NEUTRAL_LOW               100
#define SERVO_MAX_LOW                   200
#define SERVO_LOW_SCALAR                40

#define SERVO_RAW_MIN                   8000
#define SERVO_RAW_NEUTRAL               12000
#define SERVO_RAW_MAX                   16000

#define SERVO_DIR_PORT                  DDRD
#define SERVO_PORT                      PORTD
#define SERVO_A_PORT                    (1<<2)
#define SERVO_B_PORT                    (1<<3)

#define SERVO_CLOCK_CTL_REG             TCCR1B
#define SERVO_CLOCK_ENABLE              (1<<CS10)

#define SERVO_INT_PORT                  TIMSK1
#define SERVO_A_INT_ENABLE              (1<<OCIE1A)
#define SERVO_B_INT_ENABLE              (1<<OCIE1B)

#define SERVO_A_TOP_VAL                 OCR1A
#define SERVO_B_TOP_VAL                 OCR1B

#define SERVO_INTERRUPT_FLAG_PORT       TIFR1
#define SERVO_A_FLAG                    OCF1A
#define SERVO_B_FLAG                    OCF1B


////////////


int ADCsingleREAD(uint8_t adctouse)
{
    uint16_t ADCval;

    ADMUX = adctouse;         // use #1 ADC - page 261 on datasheet
    ADMUX |= (1 << REFS0);    // use AVCC as the reference
    ADMUX &= ~(1 << ADLAR);   // clear for 10 bit resolution
    
    ADCSRA|= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);    // 128 prescale for 8Mhz (SLOW!) 
    ADCSRA|= (1 << ADPS1);    

    ADCSRA |= (1 << ADEN);                                  // Enable the ADC
    ADCSRA |= (1 << ADSC);                                  // Start the ADC conversion
    while(ADCSRA & (1 << ADSC));                            // waits for the ADC to finish 

    ADCval = ADCL;
    ADCval = (ADCH << 8) + ADCval; // (| or + ?) -  ADCH is read so ADC can be updated again
    return ADCval;
 
}

/////////

void mydelay(uint16_t count) {
  while(count--) {
    _delay_ms(1);

  }
} 

/////////

void timer_init(void)
{
    // Mode 2 (CTC, OCRA as top) and 1/1024 prescaler.
    TCCR2A |= (1<<WGM21);
    TCCR2B |= ((1<<CS20)|(1<<CS21)|(1<<CS22));   

    // Set top.
    OCR2A = 156;

    // Our event timer will be normal mode with div 1024.
    TCCR0B |= ((1<<CS00)|(1<<CS02));
    TIMSK0 |= (1<<TOIE0);
}




/////////

int main(void)
{
    uint16_t ADCvalue =0;
    DDRD |= (1 << DDD6);
    // PD6 is now an output

    OCR0A = 0;    // set PWM duty cycle
    //TCCR0A |= (1 << COM0A1)|(1 << COM0A0);    // inverting mode
    TCCR0A |= (1 << COM0A1);                    // non-inverting mode

    TCCR0A |= (1 << WGM01) | (1 << WGM00);    // set fast PWM Mode
    //TCCR0B |= (1 << CS01);    // set prescaler to 8 and starts PWM
    TCCR0B |= (1 << CS02)|(1 << CS00);
;

    /* set PORTB for output*/
    DDRB = 0x08;

    while (1)
    {
        ADCvalue = ADCsingleREAD(0);
        //mydelay(ADCvalue); 
        OCR0A=ADCvalue/10;

        PORTB ^= 0x08;

    }
}







