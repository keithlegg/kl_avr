#include <avr/io.h>

#define F_CPU 16000000UL //AVR Clock Speed in MHZ
#define FOSC 16000000    // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

#include <util/delay.h>

/*
     ___    ___
 STR |1 |__|  | Vcc
   D |2       | OE
  CP |3       | QP4
 QP0 |4       | QP5
 QP1 |5       | QP6
 QP2 |6       | QP7
 QP3 |7       | QS2
 GND |8       | QS1
      ---------

-------------------------
STR        1                    strobe input
D          2                    serial data input
CP         3                    clock input
QP0 to QP7 4,5,6,7,14,13,12,11  parallel output
VSS        8                    ground supply voltage
QS1, QS2   9, 10                cascading serial outputs 
OE         15                   output enable input
VDD        16                   supply voltage


---------------------------

Notes:
`
data is shifted on the LOW-to-HIGH transitions of the CP input.
data in shift register is transferred to the storage register when the STR input is HIGH
data in storage register appears at the outputs whenever the output enable input (OE) is HIGH
LOW on OE causes the outputs to assume a high-impedance OFF-state
Operation of OE input does not affect the state of the registers
---------------------------
AVR pin assignments 

PC0   - OE
PC1   - CP
PC2   - STR
PB0   - D

*/

#define oe_pin 0x1   //pc0 
#define cp_pin 0x2   //pc1
#define str_pin 0x4  //pc2
#define data_pin 0x1 //pb0  

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


void shiftbyte_hct4094(uint8_t byte){

    PORTC ^= ~str_pin; //str low
    PORTC &= ~cp_pin;  //cp low    
    
    //serialize single byte as 8 bits 
    uint8_t i = 0;
    for (i=0; i<8; i++) {
       //set serial data bit 
       if ( !!(byte & (1 << (7 - i))) ){  // MSB
           PORTB |= data_pin; //bit is one
       }else{
           PORTB &=~ data_pin; //bit is zero  
       }

       //pulse clock - stored on rising edge  
       PORTC |= cp_pin; //clock high
       _delay_ms(10);
       PORTC &= ~cp_pin; //clock low  
    }

    //strobe sends the bits to output register
    PORTC ^= ~str_pin; //strobe low
    _delay_ms(10);   
    PORTC |= str_pin; //strobe high

}



int main()
{
    
    //set DDR
    DDRC = 0x07; //3 bits LSB
    DDRB = 0x01; //1 bit LSB 
    
    DDRD = 0xff; //1 bit LSB 


    //init pin states
    //USART_Init(MYUBRR);
    //PORTB |= clock_pin;
    //PORTB |= latch_pin;


    //set output enable HIGH 
    PORTC |= oe_pin;

    //strobe high
    PORTC |= str_pin; 

    while (1)
    {
      int delval = 200;

      shiftbyte_hct4094(0x01);
      _delay_ms(delval);
      
      shiftbyte_hct4094(0x02);
      _delay_ms(delval);
      
      shiftbyte_hct4094(0x04);
      _delay_ms(delval);
      
      shiftbyte_hct4094(0x08);
      _delay_ms(delval);

      shiftbyte_hct4094(0x10);
      _delay_ms(delval);
                     
      shiftbyte_hct4094(0x20);
      _delay_ms(delval);

      shiftbyte_hct4094(0x40);
      _delay_ms(delval);

      shiftbyte_hct4094(0x80);
      _delay_ms(delval);            

    }
   return 1;
}






