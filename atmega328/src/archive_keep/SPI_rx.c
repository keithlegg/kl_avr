#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL

/*

  this program enables SPI communication and
  Sets the AVR into Slave mode

*/


#include <avr/io.h>


int main (void)
{
    char data;

   
    DDRD |= (0xc); //2 output LEDS PD4&5

    DDRB &= ~((1<<2)|(1<<3)|(1<<5));   // SCK, MOSI and SS as inputs
    DDRB |= (1<<4);                    // MISO as output

    SPCR &= ~(1<<MSTR);                // Set as slave 
    //SPCR |= (1<<SPR0)|(1<<SPR1);       // divide clock by 128
    SPCR |= (1<<SPE);                  // Enable SPI


    while(1)
    {
        while(!(SPSR & (1<<SPIF)));    // wait until all data is received
        //data = SPDR;                   // hurray, we now have our data
        PORTD = SPDR; //SHOW DATA ON PORTD (KEITH WAS HERE)
    }
}


