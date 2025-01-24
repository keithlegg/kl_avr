#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>


#define FOSC 16000000 // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1


#define BIT_ON 0x30 //ascii 1
#define BIT_OFF 0x31 //ascii 0

volatile uint8_t stale;


/*
     this is a prototype to build a sensor->motor control 
     the UART is there to send sensor data back for debugging

    

    pinout

    PD2 -quadrature encoder A
    PD3 -quadrature encoder AMYUBRR

*/

uint16_t last_cnt    = 0;
uint16_t encoder_cnt = 100;

uint8_t encoder_dir = 0;


uint8_t enc_a = 0;
uint8_t enc_b = 0;




void USART_Init( unsigned int ubrr)
{

   UBRR0H = (unsigned char)(ubrr>>8);
   UBRR0L = (unsigned char)ubrr;
   //Enable receiver and transmitter 
   UCSR0B = (1<<RXEN0)|(1<<TXEN0);
   // Set frame format: 8data, 2stop bit 
   UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}



void USART_Transmit( unsigned char data )
{
   // Wait for empty transmit buffer 
   while ( !( UCSR0A & (1<<UDRE0)) )
   ;
   //Put data into buffer, sends the data 
   UDR0 = data;
}


/*
void read_encoder(void)
{
    enc_a = (PIND & (1 << PIND2)) == (1 << PIND2);
    enc_b = (PIND & (1 << PIND3)) == (1 << PIND3);

    //only read them if they are different 
    if (enc_a==enc_b){return;}

    //they must be different - who is higher?
    if ( !enc_a&& enc_b ) {
        encoder_cnt++;
    }else{
        encoder_cnt--;
    }
        ///////////////////////

         
          if (enc_a) {
              USART_Transmit( 0x31);
          }else{
              USART_Transmit( 0x30);
          }
          if (enc_b) {
              USART_Transmit( 0x31);
          }else{
              USART_Transmit( 0x30);
          } 
          USART_Transmit( 0x0a);
          USART_Transmit( 0x0d);
          _delay_ms(dv);  
           

}
*/
   
   









void USART_tx_string( char *data )
{
while ((*data != '\0'))
   {
      while (!(UCSR0A & (1 <<UDRE0)));
      UDR0 = *data;
      data++;
   }   
}



/***********************************************/
//for printing internal 16 bit numbers - all serial related I/O is bytesX2
void send_txt_2bytes( uint16_t data, uint8_t use_newline,  uint8_t use_space){
   uint8_t i = 0;

   for (i=0; i<=15; i++) {
       //if (i==8){  USART_Transmit(0x20); }//middle space 

       if ( !!(data & (1 << (15 - i))) ){  // MSB
           USART_Transmit( BIT_OFF );
       }else{
           USART_Transmit( BIT_ON );
       }
    }
    
    if(use_space!=0){
        USART_Transmit(0x20);    //SPACE 
    }

    if(use_newline!=0){
        USART_Transmit( 0xa ); //CHAR_TERM = new line  
        USART_Transmit( 0xd ); //0xd = carriage return
    }
}

/////////////////////////////


int main(void)
{

    USART_Init(MYUBRR);
    //DDRB = 0x08;//output LED 

    DDRD &= ~(1 << DDD2);              // Clear the PD2 pin
    DDRD &= ~(1 << DDD3);              // Clear the PD3 pin
    PORTD |= (1 << PD3)|(1 << PD2);    // turn On the Pull-up

    /*
    //Interrupt 0 Sense Control
    EICRA |= (1 << ISC00); // trigger on ANY logic change
    //Interrupt 1 Sense Control
    EICRA |= (1 << ISC10); // trigger on ANY logic change
    */

    //Interrupt 0 Sense Control
    EICRA |= (1 << ISC01); // trigger on falling edge
    //Interrupt 1 Sense Control
    EICRA |= (1 << ISC11); // trigger on falling edge

    //External Interrupt Mask Registe
    EIMSK |= (1 << INT0)|(1 << INT1);   // Turns on INT0 and INT1


    sei(); // turn on interrupts

    stale=1;


    /////
    while(1)
    {  
   
        if(stale==0)
        {
          if (enc_a) {
              encoder_dir=0;
              encoder_cnt++;
          }else if(enc_b)
          {
              encoder_dir=1;
              encoder_cnt--;
          }
          stale=1;       
        }    

        if(stale==1){
            if(encoder_cnt!=last_cnt)
            {  
                send_txt_2bytes( encoder_cnt, true, true);    
    
                //USART_Transmit(encoder_dir);     
                last_cnt= encoder_cnt; 
                 
            }   
        } 
    

    }
}


//PCINT0_vect

 
ISR (INT0_vect)
{
    enc_a = (PIND & (1 << PIND2)) == (1 << PIND2);
    enc_b = (PIND & (1 << PIND3)) == (1 << PIND3);
    stale=0;
}
 

 
ISR (INT1_vect)
{
    enc_a = (PIND & (1 << PIND2)) == (1 << PIND2);
    enc_b = (PIND & (1 << PIND3)) == (1 << PIND3);
    stale=0;
    
    //reti();

}
 



/******************************************/

/*
#define LED_ON  PORTB |= (1<<PORTB5)
#define LED_OFF PORTB &= ~(1<<PORTB5)
#define LED_TOGGLE  PINB |= (1<<PINB5)
#define SWITCH_PRESSED !(PINB & (1<<PINB7))

ISR(PCINT0_vect){
    if(SWITCH_PRESSED) //If PINB7 is low
        {    
            LED_ON;
        }
        else
        {
            LED_OFF;
        }
    
}


int main()
{
    DDRB |= (1 << PB5); // set PB5 as output pin
    DDRB &= ~(1<<DDB7); //set PB7 as an input pin
    
    PCMSK0 |= (1<<PCINT7);
    PCICR |= (1<<PCIE0);
    
    sei();
    
    while (1) {
    }
}
*/



/******************************************/
/*

uint8_t read_gray_code_from_encoder(void ) 
{ 
 uint8_t val=0; 

  if(!bit_is_clear(PIND, PD2)) 
    val |= (1<<1); 

  if(!bit_is_clear(PIND, PD3)) 
    val |= (1<<0); 

  return val; 
}

*/
