
#include <avr/io.h>
#include <avr/interrupt.h>

#include <util/delay.h>

#include "chicdoor2.h"



/***********************************/

/*

   todo:

   move software serial to hardware serial 
   use interrupts for buttons for real time feedback 
   add "pure hrbride" mode in additin to pololu controller 


*/


/***********************************/

/*
   Wiring 
   
   drdwn_sens -
   drup_sens  -
   drup_sw    - yellow
   drdwn_sw   - blue
   em_stop    - red 
   red_led    - orange
   grn_led    - brown

   switch common pwr = green 
   led common ground = black 
   sensor common pwr =  


*/



#define rxPin 3  // pin 3 connects to smcSerial TX  (not used in this example)
#define txPin 4  // pin 4 connects to smcSerial RX
// SoftwareSerial smcSerial = SoftwareSerial(rxPin, txPin);
 

 /***********************************/


// page 183 of datasheet
void USART_Init( unsigned int ubrr)
{
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    /*Enable receiver and transmitter */
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    /* Set frame format: 8data, 2stop bit */
    UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

uint8_t USART_receive(void)
{
    while (!(UCSR0A & (1 << RXC0))) {/*Busy wait.*/}
    return UDR0;
}

void USART_Transmit( unsigned char data )
{
  while ( !( UCSR0A & (1<<UDRE0)) );
  UDR0 = data;
}

void print_byte( uint8_t data){
   uint8_t i = 0;

   for (i=0; i<=7; i++) {
       //if ( !!(data & (1 << ii)) ){  // LSB
       if ( !!(data & (1 << (7 - i))) ){  // MSB
           USART_Transmit( BIT_OFF );
       }else{
           USART_Transmit( BIT_ON );
       }
    }
    
    USART_Transmit( 0xa ); //0xa = new line  
    USART_Transmit( 0xd ); //0xd = carriage return
}



void echo_uart(){
     uint8_t buf = USART_receive();
     print_byte(buf);
}

void break_connection(){
    PORTD = 0x00;    //clear port  (LEDS) 
    USART_Transmit( 0x45 ); //E
    USART_Transmit( 0x78 ); //x
    USART_Transmit( 0x69 ); //i
    USART_Transmit( 0x54 ); //T

}




// required to allow motors to move
// must be called when controller restarts and after any error
void exitSafeStart()
{
  //smcSerial.write(0x83);
}
 
// speed should be a number from -3200 to 3200
void setMotorSpeed(int speed)
{
  if (speed < 0)
  {
    //smcSerial.write(0x86);  // motor reverse command
    speed = -speed;  // make speed positive
  }
  else
  {
    //smcSerial.write(0x85);  // motor forward command
  }
  //smcSerial.write(speed & 0x1F);
  //smcSerial.write(speed >> 5);
}
 


int main(void)
{
  // initialize software serial object with baud rate of 19.2 kbps
  //smcSerial.begin(19200);
 
  // the Simple Motor Controller must be running for at least 1 ms
  // before we try to send serial data, so we delay here for 5 ms
  _delay_ms(5);
 
  // if the Simple Motor Controller has automatic baud detection
  // enabled, we first need to send it the byte 0xAA (170 in decimal)
  // so that it can learn the baud rate
  //smcSerial.write(0xAA);  // send baud-indicator byte
 
  // next we need to send the Exit Safe Start command, which
  // clears the safe-start violation and lets the motor run
  exitSafeStart();  // clear the safe-start violation and let the motor run

  while(1){
     //setMotorSpeed(3200);  // full-speed forward
     _delay_ms(1000);
     //setMotorSpeed(-3200);  // full-speed reverse
     _delay_ms(1000);
  }

  return 0;
}



