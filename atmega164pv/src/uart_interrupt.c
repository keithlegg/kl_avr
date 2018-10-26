/*
  DOES NOT WORK!!!
*/




#define F_CPU   16000000UL
#define FOSC    16000000UL

//set baudrate
#define baud 4800
#define UBBRVAL (((FOSC / (baud * 16UL))) - 1) 


#include <util/delay.h>
#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>


/*
 //FUSES SET AT EXTERNAL NO DIVIDE BY 8 

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


/**
* Inits the master timer to interrupt every 20ms. (for an 8mhz chip)
*/
void timer_init(void)
{
    // Mode 2 (CTC, OCRA as top) and 1/1024 prescaler.
    TCCR2A |= (1<<WGM21);
    TCCR2B |= ((1<<CS20)|(1<<CS21)|(1<<CS22));   

    // Set top.
    OCR2A = 156; //was 156;

    // Our event timer will be normal mode with div 1024.
    TCCR0B |= ((1<<CS00)|(1<<CS02));

    // DATA SHEET PAGE 109
    //When the TOIE0 bit is written to one, and the I-bit in the Status Register is set, the
    //Timer/Counter0 Overflow interrupt is enabled.

    TIMSK0 |= ( 1<<TOIE0 );  
}



int main()
{

    uint8_t temp;

    USART_init(UBBRVAL);    //USART_init(UBRR_2400);

    //enable interrupts (overflow timer - modified DMV 20ms @ 8mhz)
    timer_init();
    sei();

    /////

    while (1)
    {
        //temp = USART_receive();
        //USART_transmit(temp);

        USART_transmit(0x41);
        _delay_ms(100);
    }

    return 1;
}



 ISR (INT0_vect) 
 {
 
   //PORTB &= ~(1 << LED); // Set 0 on LED pin (led turn on)
   _delay_ms (1000);
 }

/*
 ISR (USART0_TX_vect) 
 {
 
   //PORTB &= ~(1 << LED); // Set 0 on LED pin (led turn on)
   _delay_ms (1000);
 }
 */



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
