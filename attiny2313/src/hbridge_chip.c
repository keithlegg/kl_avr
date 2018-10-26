#include <avr/io.h>
#include <util/delay.h>


/*

 HBRIDGE BOT
 USES L293D/sn75441ONE CHIP
 
 CONTROLLED WITH PORTB (0-4)
 ENABLE LINE - pin 4
 MOTOR CTRL 0-4 (NEVER TURN 0,1 or 2-3 ON AT SAME TIME )

  portb 0x01 = 2A
  portb 0x02 = 1A
  portb 0x04 = 3A
  portb 0x08 = 4A
  portb 0x10 = enable 1,2

*/

int main (void)
{
   DDRB |= 0b00011111; // Set LED as output
   int a = 0;
   uint8_t       bytes[4]; 
   uint16_t      delay =300; 

   bytes[0] = 0x01;
   bytes[1] = 0x02;
   bytes[2] = 0x04;
   bytes[3] = 0x08;


   while (1)
   {

      for (a=0;a<4;a++){
       PORTB = bytes[a];
       PORTB |= 0b00010000;

       _delay_ms(delay);
       PORTB = 0x00;
       _delay_ms(delay);

      }


   }
} 





