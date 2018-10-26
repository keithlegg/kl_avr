#include <avr/io.h>

#define F_CPU 16000000UL //AVR Clock Speed in MHZ
#define FOSC 16000000    // Clock Speed
#include <util/delay.h>

//---------------------
//PF---PA
//PE---PC
//---------------------
//PB---|
//PD---|
//---------------------

void blink_port(volatile uint8_t *ddrport, volatile uint8_t *port){
    *ddrport = 0xff;
    uint8_t delay_ms = 200;
    uint8_t i = 0;
    for(i=0;i<8;i++){
       *port = (1 << i);
       _delay_ms(delay_ms);         
    }
    *port = 0x00;
}

/*******************/

int main (void)
{
   while(1){
       blink_port(&DDRF, &PORTF);
       blink_port(&DDRA, &PORTA);
       blink_port(&DDRE, &PORTE);
       blink_port(&DDRC, &PORTC);
       blink_port(&DDRB, &PORTB);
       blink_port(&DDRD, &PORTD);                     
   }

} 




