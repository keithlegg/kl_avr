#include <avr/io.h>

#define F_CPU 16000000UL //AVR Clock Speed in MHZ
#define FOSC 16000000    // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

#include <util/delay.h>

#define latch_pin 0x1 //Arduino D8  - NES Orange wire
#define clock_pin 0x2 //Arduino D9  - NES Red wire
#define data_pin 0x4  //Arduino D10 - NES Yellow

// #define sbi(a, b) (a) |= (1 << (b))
// #define cbi(a, b) (a) &= ~(1 << (b)) 

void USART_Init( unsigned int ubrr)
{

  UBRR0H = (unsigned char)(ubrr>>8);
  UBRR0L = (unsigned char)ubrr;
  /*Enable receiver and transmitter */
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);
  /* Set frame format: 8data, 2stop bit */
  UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void USART_Transmit( unsigned char data )
{
  /* Wait for empty transmit buffer */
  while ( !( UCSR0A & (1<<UDRE0)) )
  ;
  /* Put data into buffer, sends the data */
  UDR0 = data;
}

void USART_tx_string( char *data )
{
   while ((*data != '\0'))
   {
      while (!(UCSR0A & (1 <<UDRE0)));
      UDR0 = *data;
      data++;
   }   
}


void print_bit( uint8_t data){
    if ( data == 1 ){  
        USART_Transmit( 0x31 );
    }else{
        USART_Transmit( 0x30 );
    }
    USART_Transmit( 0xa ); //0xa = new line  
    USART_Transmit( 0xd ); //0xd = carriage return
}

void print_byte( uint8_t data){
   uint8_t ii = 0;

   for (ii=0; ii<=7; ii++) {

       //if ( !!(data & (1 << ii)) ){  // LSB
       if ( !!(data & (1 << (7 - ii))) ){  // MSB
           USART_Transmit( 0x31 );
       }else{
           USART_Transmit( 0x30 );
       }
    }
    USART_Transmit( 0xa ); //0xa = new line  
    USART_Transmit( 0xd ); //0xd = carriage return
}

 
void controllerRead(void) {
  uint8_t controller_data = 0;
  uint8_t i = 0;
  
  PORTB &= ~latch_pin; 
  PORTB &= ~clock_pin; 
  PORTB |= latch_pin;  
  _delay_ms(200); //_delay_us(2);// _delay_ms(200);
  PORTB &= ~latch_pin; 
  controller_data = !!(PINB & data_pin);

  for (i=1; i<=7; i++) {
     PORTB |= clock_pin;
    _delay_ms(200);// _delay_us(2);//  _delay_ms(200);
     controller_data = controller_data << 1;
     controller_data = controller_data + !!(PINB & data_pin); 
     _delay_us(400); //_delay_us(4);//  _delay_us(400);
     PORTB &= ~clock_pin;
  }

  print_byte( controller_data);

}


int main(){
  DDRB = 0xFB; 
  USART_Init(MYUBRR);
  PORTB |= clock_pin;
  PORTB |= latch_pin;
  while (1)
  {
    controllerRead();
  }
 return 1;
}
