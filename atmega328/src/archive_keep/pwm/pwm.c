
#include <avr/io.h>


void PWMInit()
{
	DDRD |=  (1 << 6);  
	TCCR0A = (1<<WGM00)|(1<<COM0A1); //  Timer/Counter   8-Bit PWM
	TCCR0B = (1<<CS01) | (1<<CS00);   
}

void setPWM(uint8_t pwmWert)
{

	OCR0A = pwmWert;
}
