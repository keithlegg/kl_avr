#include <avr/io.h>
//#include <avr/interrupt.h>

#define F_CPU 16000000UL //AVR Clock Speed in MHZ
#define FOSC 16000000    // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

#include <util/delay.h>



/*


Mapping

#CLK -PC2 0x04
#RW - PC1 0x02
#CS - PC0 0x01


#Addr
0 -PB0
1 -PB1
2 -PB2
3 -PB3
4 -PB4

#Data
PortD


*/

/*

void wrSID (unsigned char port, unsigned char data) {
  // Send byte 'data' to address 'port'.
  // SIDdelay is inserted between operations for settling time. 
  
  PORTB = (port & 0x0F) | 0x10;  // Lower bit, keep CS deactive
  PORTD = ((port | 0x20) & 0xF0) >> 2; // Upper bit, reset high
  PORTD |= 0x80;    // Clock in Address
  SIDdelay();
  PORTD &= ~(0x80);
  SIDdelay();

  PORTB = (data & 0x0F) | 0x10;  // Lower bit, keep CS deactive
  PORTD = (data & 0xF0) >> 2;    // Upper bit
  PORTB &= ~(0x10);   // Activate /CS
  SIDdelay();
  PORTB |= 0x10;      // Deactivate /CS
  SIDdelay();
}  
*/

void SIDdelay () {
  // Make a really short delay in software to allow for settling
  // time on the bus. The loop needs the NOP command because the
  // compiler optimisations will detect nothing is happening
  // witout it and consequently not put in a loop at all.

  unsigned char i;
  for(i=0;i<10;i++) __asm("nop\n\t");
}


void wrSID(uint8_t addr, uint8_t data){
   //mask off addr ?
   //uint8_t addr_mask = 0x1f

   // SET Addr 
   PORTB = addr;

   //CLOCK HIGH 
   PORTC |= 0x04;   
   SIDdelay();


   //clock low
   PORTC &= ~0x04;   
   SIDdelay();

   /////////////////////////////

   //SET Data
   PORTD = data;

   //CLOCK HIGH 
   //PORTC |= 0x04;   
   //_delay_us(delayval);

   //clock low
   //PORTC &= ~0x04;   
   //_delay_us(delayval);

   //////////////////////////////////
   //CS low - Activate 
   DDRC &= ~(0x4); 
   SIDdelay();

   // CS high - Activate 
   DDRC |= (0x4); 
   SIDdelay();

}


/////////////////
void init_SID(){

   //start all low ?
   PORTB = 0x00;
   PORTC = 0x00;
   PORTD = 0x00;

   //CS high (disabled)
   DDRC |= (0x4); 

   int c = 0;

   // Clear the SID
   //for(c=0; c<25;c++)
   //  wrSID(c,0);

}

int main (void)
{

   DDRB = 0x1f; 
   DDRC = 0x07; 
   DDRD = 0xff;

   //DDRD |=  (0x4); // On 
   //DDRD &= ~(0x4); // Off

   init_SID();


/*
FROM GERMAN GUY

1> 00001 10000010   
2> 00101 00001001  
3> 00110 10000000  
4> 11000 00001111   
5> 00100 00010001   

*/


   while (1)
   {


      wrSID(0b00000001 , 0b10000010 );  // Set Frequency of voice 1
      wrSID(0b00000101 , 0b00001001 );  // Set Attack/Decay of voice 1
      wrSID(0b00000110 , 0b10000000 );  // Set Sustain level of voice 1
      wrSID(0b00011000 , 0b00001111 );  // Set maximum Volume
      wrSID(0b00000100 , 0b00010001 );  // Set Triangle waveform and Gate bits                

      /* 
      // Write some default values to the SID
       wrSID(24,15);    // Turn up the volume
       wrSID(5,0);      // Fast Attack, Decay
       wrSID(6,0xF0);   // Full volume on sustain, quick release
       wrSID(2,2048&0xFF);
       wrSID(3,2048>>8);
       wrSID(4,0x21);   // Enable gate, sawtooth waveform.
       */

      _delay_ms(500);

   }
} 

