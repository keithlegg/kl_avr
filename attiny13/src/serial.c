
 #include <avr/io.h>
 #include <avr/interrupt.h>
 
 #define SD_DATAIN PB0 // Data in
 #define SD_CS PB1 // Chip select
 #define SD_CLK PB2 // Clock
 #define LED PB4 // Led (indicate the value of data)
 
 int i;
 int s;
 volatile int Data;
 
 int main(){
 
 GIMSK = _BV (INT0); // int - Enable external interrupts int0
 MCUCR = _BV (ISC01); // int - INT0 is falling edge
 sei(); // int - Global enable interrupts
 
 Data = 150; // initilize data register to 150
 
 DDRB &= ~(1<<SD_DATAIN); // Set direction register output Datain
 DDRB &= ~(1<<SD_CLK); // Set direction register output clock
 DDRB |= (1 << LED); // Set direction register output
 
 for (;;) // Main loop (endless)
 
 {
 for (s = 0 ; s < 255 ; s++) // /master loop show the value
 { // /(brighness) of data input.
 
 if (Data > s)
 {
 PORTB &= ~(1 << LED); // Set 0 on LED pin (led turn on)
 }
 else
 {
 PORTB |= (1 << LED); // Set 1 on LED pin (led turn off)
 }
 }}
 return 0;
 }
 
 ISR (INT0_vect) // Interrupt on Int0 vector
 {
 
 for (i = 0 ; i < 8 ; i++) // For loop 0 to 7
 {
 while (bit_is_clear(PINB,SD_CLK)); // Rising edge detector on Serial Clk
 
 if bit_is_set(PINB,SD_DATAIN) // Bit detector on Sd_Data input
 {
 Data ^= (1<<i); // Update data register (one bit by one) 8 time
 }
 
 while (bit_is_set(PINB,SD_CLK)); // Faling edge detector on Serial Clk
 }
 
 }
