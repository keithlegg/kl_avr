#include <avr/io.h>
#include <util/delay.h>

#define F_CPU = 16000000



// this program enables SPI communication and
// Sets the AVR into Master mode
//MASTER CODE 

int main (void)
{
    char data;

    DDRB |= (1<<2)|(1<<3)|(1<<5);    // SCK, MOSI and SS as outputs
    DDRB &= ~(1<<4);                 // MISO as input

    SPCR |= (1<<MSTR);               // Set as Master
    SPCR |= (1<<SPR0)|(1<<SPR1);     // divided clock by 128
    SPCR |= (1<<SPE);                // Enable SPI
    
    while(1)
    {
        SPDR = data;                 // send the data
        while(!(SPSR & (1<<SPIF)));  // wait until transmission is complete

        // if you have multiple slaves, this is where you want to switch
    }
}

////////////////
/*


//SLAVE CODE 

// this program enables SPI communication and
// Sets the AVR into Slave mode


#include <avr/io.h>


int main (void)
{
    char data;

    DDRB &= ~((1<<2)|(1<<3)|(1<<5));   // SCK, MOSI and SS as inputs
    DDRB |= (1<<4);                    // MISO as output

    SPCR &= ~(1<<MSTR);                // Set as slave 
    SPCR |= (1<<SPR0)|(1<<SPR1);       // divide clock by 128
    SPCR |= (1<<SPE);                  // Enable SPI

    while(1)
    {
        while(!(SPSR & (1<<SPIF)));    // wait until all data is received
        data = SPDR;                   // hurray, we now have our data
    }
}

*/

