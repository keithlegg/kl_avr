#include <avr/io.h>


#define F_CPU 16000000UL
#define FOSC 16000000UL

#include <util/delay.h>

/*

 this program enables SPI communication and
 Sets the AVR into Master mode

*/

int main (void)
{
    char data  = 0x2;//0b00001000;
    char data2 = 0x4;//0b00001100;

    DDRB |= (1<<2)|(1<<3)|(1<<5);    // SCK, MOSI and SS as outputs
    DDRB &= ~(1<<4);                 // MISO as input

    SPCR |= (1<<MSTR);               // Set as Master
    //SPCR |= (1<<SPR0)|(1<<SPR1);     // divided clock by 128
    SPCR |= (1<<SPE);                // Enable SPI
    
    while(1)
    {
        SPDR = data;                 // send the data
        while(!(SPSR & (1<<SPIF)));  // wait until transmission is complete
        _delay_ms(1000);

        SPDR = data2;                 // send the data
        while(!(SPSR & (1<<SPIF)));  // wait until transmission is complete
        _delay_ms(1000);

        // if you have multiple slaves, this is where you want to switch
    }
}

