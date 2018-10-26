#include <avr/io.h>
#define F_CPU 16000000UL

#define USART_BAUDRATE 4800
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

int main (void)
{
   char ReceivedByte;

   UCSR0B |= (1 << RXEN0) | (1 << TXEN0);   // Turn on the transmission and reception circuitry
   //UCSR0C |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1); // Use 8-bit character sizes

   // UBRR0H = (uint8_t)(BAUD_PRESCALE>>8); 
   // UBRR0L = (uint8_t)BAUD_PRESCALE;      

     UBRR0H = (BAUD_PRESCALE >> 8);           // Load upper 8-bits of the baud rate value into the high byte of the UBRR register
     UBRR0L = BAUD_PRESCALE;                  // Load lower 8-bits of the baud rate value into the low byte of the UBRR register

   for (;;) // Loop forever
   {
      while ((UCSR0A & (1 << RXC0)) == 0) {};  // spinlock read
      ReceivedByte = UDR0;                     // Fetch the received byte value into the variable "ByteReceived"

      while ((UCSR0A & (1 << UDRE0)) == 0) {}; // spinlock write
      UDR0 = ReceivedByte;                     // Echo back the received byte back to the computer


   }   
} 
