/*
  ported from my 328p code to 164p 

*/

#include <avr/io.h>

//UART SETUP 
#define baud 57600 //set baudrate for RPI
#define baud2 4800 //set baudrate2 for GPS
#define FOSC 16000000UL
#define UBBRVAL FOSC/16/baud-1
#define UBBRVAL2 FOSC/16/baud2-1

#define BUFFSIZE 10

#include <util/delay.h>

/*
  NES CONTROLLER WIRING 
  white   - +5
  red     - clock
  orange  - latch
  yellow  - data 
  brown   - ground   
*/

//NES SETUP 
#define data_pin 0x1  // NES Yellow  
#define latch_pin 0x2 // NES Orange wire
#define clock_pin 0x4 // NES Red wire

//global declared out here
volatile uint8_t bufferidx =0;
volatile uint8_t buffer[BUFFSIZE];

// #define sbi(a, b) (a) |= (1 << (b))
// #define cbi(a, b) (a) &= ~(1 << (b)) 

/***********************/

void send_string_0(char s[])
{
   int i =0;
   
   while (s[i] != 0x00)
   {
      USART_transmit_0(s[i]);
      i++;
   }
} 

void USART_init_0(uint16_t ubrr)
{
    // Load upper and lower bytes of baud rate into the UBRR register
    UBRR0H = (uint8_t)(ubrr>>8); 
    UBRR0L = (uint8_t)ubrr;      
    /*Enable receiver and transmitter */
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    /* Set frame format: 8data, 2stop bit */
    //UCSR0C = (1<<USBS0)|(3<<UCSZ00);    
}

void USART_transmit_0(uint8_t data)
{
    while (!(UCSR0A & (1 << UDRE0))) {/*Busy wait.*/}
    UDR0 = data;
}


uint8_t USART_receive_0(void)
{
    while (!(UCSR0A & (1 << RXC0))) {/*Busy wait.*/}
    return UDR0;
}

///////////////

void USART_init_1(uint16_t ubrr)
{
    UBRR1H = (uint8_t)(ubrr>>8); 
    UBRR1L = (uint8_t)ubrr;     

    // Enable transmitter and receiver.
    UCSR1B |= ((1 << RXEN1)|(1 << TXEN1));
}


void USART_transmit_1(uint8_t data)
{
    while (!(UCSR1A & (1 << UDRE1))) {/*Busy wait.*/}
    UDR1 = data;
}


uint8_t USART_receive_1(void)
{
    while (!(UCSR1A & (1 << RXC1))) {/*Busy wait.*/}
    return UDR1;
}

void print_bit( uint8_t data){
    if ( data == 1 ){  
        USART_transmit_0( 0x31 );
    }else{
        USART_transmit_0( 0x30 );
    }
    USART_transmit_0( 0xa ); //0xa = new line  
    USART_transmit_0( 0xd ); //0xd = carriage return
}

void print_byte( uint8_t data){
   uint8_t ii = 0;

   for (ii=0; ii<=7; ii++) {

       //if ( !!(data & (1 << ii)) ){  // LSB
       if ( !!(data & (1 << (7 - ii))) ){  // MSB
           USART_transmit_0( 0x31 );
       }else{
           USART_transmit_0( 0x30 );
       }
    }
    USART_transmit_0( 0xa ); //0xa = new line  
    USART_transmit_0( 0xd ); //0xd = carriage return
}

/******************************/
uint8_t controllerRead(void) {
  uint8_t controller_data = 0;
  uint8_t i = 0;
  uint8_t latchtime = 30;//u_sec delay between clock pulses 

  PORTB &= ~latch_pin; 
  controller_data = !!(PINB & data_pin); //NES yellow wire

  for (i=0; i<7; i++) {
     PORTB |= clock_pin;
     _delay_us(latchtime); 
     controller_data = controller_data << 1;
     controller_data = controller_data + !!(PINB & data_pin); 
     PORTB &= ~clock_pin;
     _delay_us(latchtime); 
  }
  
  PORTB |= latch_pin;  

  return controller_data;
}
/******************************/
void gameLoop(){
  uint8_t nesctrlr =0;
  nesctrlr = controllerRead(); //read a byte from the controller 
  PORTA = nesctrlr;
  if (PORTA & 0x01){
     PORTC ^= 0x01;
  }


}
/******************************/
int main()
{

    USART_init_0(UBBRVAL);    
    //USART_init_1(UBBRVAL2);  
   
    DDRB = 0xff; 
    DDRB |= latch_pin; 
    DDRB |= clock_pin; 
    DDRB &= ~data_pin;  

    DDRC = 0xff;
    
    DDRA = 0xff;
    
    PORTB |= 0x8; //is this a good idea ? power from MCU?
    while (1)
    {
        gameLoop();
    }

    return 1;
}



