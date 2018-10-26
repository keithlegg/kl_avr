#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL //AVR Clock Speed in MHZ
#define FOSC 16000000    // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

#include <util/delay.h>


volatile uint16_t tick; 
volatile uint16_t do_transmit; 


//PB5 - d13

// #define sbi(a, b) (a) |= (1 << (b))
// #define cbi(a, b) (a) &= ~(1 << (b)) 

void USART_Init( unsigned int ubrr)
{

  UBRR0H = (unsigned char)(ubrr>>8);
  UBRR0L = (unsigned char)ubrr;
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);
  UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void USART_Transmit( unsigned char data )
{
  while ( !( UCSR0A & (1<<UDRE0)) )
  ;
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
   USART_Transmit( 0xa ); //0xa = new line  
   USART_Transmit( 0xd ); //0xd = carriage return

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
 


int main(){
  DDRB = 0x20;//output LED  - D13

  DDRD &= ~(1 << DDD2);     // Clear the PD2 pin
  PORTD |= (1 << PORTD2);   // turn On the Pull-up

  EICRA |= (1 << ISC00);    // set INT0 to trigger on ANY logic change
  EIMSK |= (1 << INT0);     // Turns on INT0
  sei();                    // turn on interrupts
  //timer stuff
  TCCR1B = 1; //start Timer1
  
  //set prescaler - target pulses are ~1ms 
  //TCCR0B |= (1 << CS01) | (1 << CS00); //prescale to 1/64 FOSC
  //TCCR0B |= (1 << CS02) ; //prescale to 1/256 FOSC


  //JUST FOR CALCULATION EXAMPLE AT 8MHZ
  // XXXTCCR0B = (1<<CS01) | (1<<CS00);   // Clock/64, 1/(8000000/64)= 0.000008 seconds per tick
  // XXXOCR0A = 200;      //   0.000008 *230 = 1.6 ms
  // XXXOCR0B = 100;      //     0.8 ms

  USART_Init(MYUBRR);
 
  //char x[3] = {0x41,0x42,0x43};
  //USART_tx_string(x);

  while (1)
  {
     if (do_transmit){
        print_byte(tick);
     } 
  }
 return 1;
}



ISR (INT0_vect)
{
   //PORTB ^= 0x08;  //TO TEST 
   tick=TCNT1; //16 bit timer 
   
   
   //WE HAVE A CAPTURE VALUE! = tick
   if (tick<5000)
   {
      PORTB |= 0x20;
      do_transmit = 1;
   }else{
      PORTB &= ~0x20;
      do_transmit = 0;      
   }
   



   TCNT1=0;
}




