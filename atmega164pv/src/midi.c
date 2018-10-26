#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>



#define F_CPU   16000000UL

//set baudrate
#define baud 31250
#define UBBRVAL (((F_CPU  / (baud * 16UL))) - 1) 

/*
 MIDI TEST
*/


const uint8_t notes[8] ={72,73,74,75,76,77,78,79};
const int numnotes = 8;






/*

 //this works for 9600
 #define UBRR_2400   103UL

 #include <avr/power.h>
 clock_prescale_set(clock_div_1); // add this in the start of main 
*/


/**
 * Enables the USART with the given UBRR.
 */
static void USART_init(uint16_t ubrr);

/**
 * Synchronously transmits one character.
 */
static void USART_transmit(uint8_t data);

/**
 * Synchronously receives one character.
 */
static uint8_t USART_receive(void);




static void playMidiNote(uint8_t channel,uint8_t note,uint8_t velocity){
  uint8_t midiMesage=0x90 + (channel-1);
  USART_transmit(midiMesage);
  USART_transmit(note);
  USART_transmit(velocity);
 
}



static void USART_init(uint16_t ubrr)
{
    UBRR0H = (uint8_t)(ubrr>>8);
    UBRR0L = (uint8_t)ubrr;

    // Enable transmitter and receiver.
    UCSR0B |= ((1 << RXEN0)|(1 << TXEN0));
}


static void USART_transmit(uint8_t data)
{
    while (!(UCSR0A & (1 << UDRE0))) {/*Busy wait.*/}
    UDR0 = data;
}


static uint8_t USART_receive(void)
{
    while (!(UCSR0A & (1 << RXC0))) {/*Busy wait.*/}
    return UDR0;
}




int main()
{

    //uint8_t temp;

    USART_init(UBBRVAL);    //USART_init(UBRR_2400);

    /////
   
    while (1)
    {
        //temp = USART_receive();
       // USART_transmit(temp);
      for (uint8_t noteNumber=0;noteNumber<numnotes;noteNumber++)
      {
	    playMidiNote(1,notes[noteNumber],64);
	    _delay_ms(70);
	    playMidiNote(1,notes[noteNumber],0);
	    _delay_ms(30);   

      }

    }

    return 1;
}


