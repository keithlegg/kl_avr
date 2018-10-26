#include <avr/io.h>
#define F_CPU 16000000UL

#define F_CPU   16000000UL
#define FOSC    16000000UL

#include <avr/interrupt.h>
#include <avr/delay.h>

#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((FOSC / (USART_BAUDRATE * 16UL))) - 1)


int main (void)
{
   char ReceivedByte;
   DDRB |= (1 << 0); // Set LED as output


   UCSR0B |= (1 << RXEN0) | (1 << TXEN0);   // Turn on the transmission and reception circuitry

   UBRR0H = (BAUD_PRESCALE >> 8);           // Load upper 8-bits of the baud rate value into the high byte of the UBRR register
   UBRR0L = BAUD_PRESCALE;                  // Load lower 8-bits of the baud rate value into the low byte of the UBRR register

   //INTERRUPT STUFF
   UCSR0B |= (1 << RXCIE0); // Enable the USART Recieve Complete interrupt (USART_RXC)
   sei();                   // Enable the Global Interrupt Enable flag so that interrupts can be processed 


   for (;;) // Loop forever
   {
      while ((UCSR0A & (1 << RXC0)) == 0) {};  // spinlock read
      ReceivedByte = UDR0;                     // Fetch the received byte value into the variable "ByteReceived"

      //NON INTERRUPT ECHO 
      //while ((UCSR0A & (1 << UDRE0)) == 0) {}; // spinlock write
      //UDR0 = ReceivedByte;                     // Echo back the received byte back to the computer
      
   }   
} 


//INTERRUPT ECHO 
ISR(USART0_RX_vect)
{
        PORTB ^= (1 << 0); // Toggle the LED

        char ReceivedByte = UDR0; // Fetch the recieved byte value into the variable "ByteReceived"
        UDR0 = ReceivedByte;      // Echo back the received byte back to the computer
} 



