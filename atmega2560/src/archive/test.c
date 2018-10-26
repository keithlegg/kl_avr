#include <avr/io.h>

#define F_CPU 16000000UL //AVR Clock Speed in MHZ
#define FOSC 16000000    // Clock Speed
int main (void)
{
    DDRD = 0xff;     
    while (1)
    {
        PORTD ^= 0xff;
   }
} 




