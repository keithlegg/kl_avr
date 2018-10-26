#include <avr/io.h>

#define F_CPU 16000000UL // AVR Clock Speed in MHZ
#define FOSC 16000000    // Clock Speed

#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>


#define PWM_PRESCALLER 64
#define ICR_MAX (long double)F_CPU/PWM_PRESCALLER/50
#define OCR_MIN ICR_MAX/20
#define OCR_MAX ICR_MAX/10


volatile unsigned long adc_val=0;
volatile unsigned long counter=0;
unsigned long curr_adc=0;



/*

  SERVO PWM 

  20MS Period / 50Hz 

  Center pos = 1500us - 1.5 MS
  +90        = 2000us - 2   MS 
  -90        = 1000us - 1   MS 


  // description of servo PWM - may or may not be what this code is doing. 

  timer1 (16 bit) set to "fast pwm" mode with top=ICR1
  set ICR1 to 20000
  timer's triggering clock is 1MHZ = 8MHZ/8

  this way you get frequency of 50 hz  
  and the ocr is set to exactly what you want the high pulse to be
  to center the servo

  OCR1=1500;


  // WIRING RELATED 
  // the signal will be output on PB5/OC1A
  // COLORS 

  Black or Brown is for ground.
  Red is for power (~4.8-6V).
  Yellow, Orange, or White is the signal wire (3-5V).

*/


void pwm_write (uint16_t val)
{
    OCR1A = val;
}


/******************************/

int main (void)
{
        
  
  DDRB = 0xff;
  
  ICR1 = ICR_MAX;  // Input Capture Register (sets the period)
  OCR1A = OCR_MIN; // Output Capture Register
  TCCR1A = (1 << COM1A1) | (1<<WGM11);
  TCCR1B = (1<<WGM13) | (1<<WGM12) | (1<<CS11) | (1<<CS10);
    
  
  //ADMUX = (1 << REFS0);
  //ADCSRA = (1 << ADEN)|(1 << ADFR)|(1 << ADIE)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
  //sei();
  //ADCSRA |= (1<<ADSC);
  
  while(1)
  {
    /*
    if(counter >= 500)
    {
      cli();
      unsigned long round_val = round(adc_val / counter);
      adc_val=0;
      counter=0;
      if(abs(round_val - curr_adc) > 1)
      {
        curr_adc = round_val;
      }
      long double ocr = OCR_MIN + ((long double)curr_adc * (OCR_MAX - OCR_MIN)/1024);
      // sprintf(uart_s, "ADC = %8lu, OCR = %f, rounded = %d\r\n", curr_adc, ocr, (int)round(ocr));
      //uart_send_string((unsigned char*) uart_s); 
      OCR1A = (int)round(ocr);
      //_delay_ms(100);
      sei();
    }
    */
    uint16_t cnt = 0;
    for(cnt=0;cnt<1024;cnt++){
        pwm_write(cnt);
        _delay_ms(5);
    }

  }
} 




/*
ISR (ADC_vect)
{
  adc_val += ADC;
  counter++;
}
*/

