#include <avr/io.h>
#include <util/delay.h>


#define HC595_PORT PORTB
#define HC595_DDR DDRB
#define HC595_DS_POS PB0     // Data pin    (DS)    - (blue)
#define HC595_SH_CP_POS PB1  // Shift Clock (SH_CP) - (yellow)
#define HC595_ST_CP_POS PB2  // Store Clock (ST_CP) - (green)

//Low level macros to change data (DS)lines
#define HC595DataHigh() (HC595_PORT|=(1<<HC595_DS_POS))
#define HC595DataLow() (HC595_PORT&=(~(1<<HC595_DS_POS)))

/*
  74HC595 WIRING 
       __   __  
  out1-| +_+ |-Vcc 
  out2-|     |-out0
  out3-|     |-HC595_DS_POS/DATA
  out4-|     |-OE/GND
  out5-|     |-RCLK/HC595_SH_CP_POS/CLOCK
  out6-|     |-SRCLK/HC595_ST_CP_POS/LATCH  
  out7-|     |-SRCLR/VCC
  GND -|_____|-NC/ CARRY BIT 

*/

/*********************/
void HC595Init()
{
   //Make the Data(DS), Shift clock (SH_CP), Store Clock (ST_CP) lines output
   HC595_DDR|=((1<<HC595_SH_CP_POS)|(1<<HC595_ST_CP_POS)|(1<<HC595_DS_POS));
}

/*********************/
//Sends a clock pulse on SH_CP line
void HC595Pulse()
{
   //Pulse the Shift Clock
   HC595_PORT|=(1<<HC595_SH_CP_POS);//HIGH
   _delay_ms(1);   
   HC595_PORT&=(~(1<<HC595_SH_CP_POS));//LOW
   _delay_ms(1);  
}
/*********************/
//Sends a clock pulse on ST_CP line
void HC595Latch()
{
   //Pulse the Store Clock
   HC595_PORT|=(1<<HC595_ST_CP_POS);//HIGH
   _delay_ms(1);

   HC595_PORT&=(~(1<<HC595_ST_CP_POS));//LOW
   _delay_ms(1);
}
/*********************/
void HC595Write(uint8_t data)
{
   //Order is MSB first
   for(uint8_t i=0;i<8;i++)
   {
      //Output the data on DS line according to the
      //Value of MSB
      if(data & 0b10000000)
      {
          HC595DataHigh();//MSB is 1 so output high
      }
      else
      {
          HC595DataLow();//MSB is 0 so output high
      }

      HC595Pulse();  //Pulse the Clock line
      data=data<<1;  //Now bring next bit at MSB position
      //_delay_ms(50); 

   }

   //Now all 8 bits have been transferred to shift register
   //Move them to output latch at one
   HC595Latch();
}
/*********************/
void main()
{
                    
   HC595Init();

   while(1)
   {
      HC595Write(0b10000000 );
      _delay_ms(10);
      HC595Write(0b01000000 );
      _delay_ms(10);
      HC595Write(0b00100000 );
      _delay_ms(10);
      HC595Write(0b00010000 );
      _delay_ms(10);
      HC595Write(0b00000100 );
      _delay_ms(10);
      HC595Write(0b00000010 );
      _delay_ms(10);
      HC595Write(0b00000001 );
      _delay_ms(10);
   
   }
}


   /* 
   uint8_t led_pattern[8]={
                        0b10000001,
                        0b11000011,
                        0b11100111,
                        0b11111111,
                        0b01111110,
                        0b00111100,
                        0b00011000,
                        0b00000000,
                     };

      for(uint8_t i=0;i<8;i++)
      {
         HC595Write(led_pattern[i]);   //Write the data to HC595
      } 

   */
