#include "../include/main.h"

/*
        ***************************************
	Keith Legg 

	first start of a GPS robot brain 
	made for atmega164p

	PORT A,B,C is for talking to peripheral board
	PORT D is for UARTS and LED heartbeats 

	UART0 is connected to raspberry pi at 9600 baud
	UART1 is connected to GPS at 4800 baud
        ***************************************
*/


int main()
{

    uint8_t temp;
  
    USART_init_0(UBBRVAL);    
    USART_init_1(UBBRVAL2);  

    //heartbeat LEDs
    DDRD = 0b11110000;

    
    //USE UART0
    while (1)
    {
        //need to move these to uart complete ISR so we can blink heartbeat
        temp = USART_receive_1(); //read UART 1   //GPS
        USART_transmit_0(temp);   //write UART 0  //RPI 
        //_delay_ms(1000);

    }


    /*
        //temp = USART_receive_1();
        USART_transmit_1(0x61); //letter a 
        _delay_ms(1000);
    */


    return 1;
}

///////////////


static void USART_init_0(uint16_t ubrr)
{
    // Load upper and lower bytes of baud rate into the UBRR register
    UBRR0H = (uint8_t)(ubrr>>8); 
    UBRR0L = (uint8_t)ubrr;      

    // Enable transmitter and receiver.
    UCSR0B |= ((1 << RXEN0)|(1 << TXEN0));
}


static void USART_transmit_0(uint8_t data)
{
    while (!(UCSR0A & (1 << UDRE0))) {/*Busy wait.*/}
    UDR0 = data;
}

static uint8_t USART_receive_0(void)
{
    while (!(UCSR0A & (1 << RXC0))) {/*Busy wait.*/}
    return UDR0;
}

///////////////

static void USART_init_1(uint16_t ubrr)
{
    UBRR1H = (uint8_t)(ubrr>>8); 
    UBRR1L = (uint8_t)ubrr;     

    // Enable transmitter and receiver.
    UCSR1B |= ((1 << RXEN1)|(1 << TXEN1));
}


static void USART_transmit_1(uint8_t data)
{
    while (!(UCSR1A & (1 << UDRE1))) {/*Busy wait.*/}
    UDR1 = data;
}


static uint8_t USART_receive_1(void)
{
    while (!(UCSR1A & (1 << RXC1))) {/*Busy wait.*/}
    return UDR1;
}

