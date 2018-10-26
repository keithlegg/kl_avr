 
 
 #define F_CPU 1200000 // clock 1.2MHz, internal oscillator
 
 #include <avr/io.h>
 #include <util/delay.h>
 #include <avr/interrupt.h>
 
 #define LED PB4
 
 int i;
 int Data ;
 
 
 int main(void) {
 
 
 GIMSK = _BV (INT0); // int - Enable external interrupts int0
 MCUCR = _BV (ISC01); // int - INT0 is falling edge
 sei(); // int - Global enable interrupts
 
 DDRB |= (1 << LED); // Set direction register output
 
 for (;;) // loop (endless)
 {
 PORTB |= (1 << LED); // Set 1 on LED pin (led turn off)
 }
 
 return 0;
 }
 
 ISR (INT0_vect) // Interrupt on Int0 vector
 {
 
 PORTB &= ~(1 << LED); // Set 0 on LED pin (led turn on)
 _delay_ms (1000);
 }
 
