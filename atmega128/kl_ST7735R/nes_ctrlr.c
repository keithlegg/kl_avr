
#include "nes_ctrlr.h"

/*

  CONTROLLER CODES
    0xFF - nothing pressed 
    0xF7 - up 
    0xFD - left 
    0xFB - down 
    0xF7 - up 

    0xF6 - right/up
    0xFA - right/down
    0xF5 - left/up      
    0xF9 - left/down  
    
    0xFE - right 
    
    //mask of high bit for buttons other than direction 
    0xDF - select 
    0xEF - start
    0xBF - B button 
    0x7F - A button 

    0x3X - both A,B buttons 

*/

void NES_CTRLR_Init( void )
{
    nes_gpio_ddr = 0xff;
    nes_gpio_ddr &=~ nes_data_pin; //make data pin input
}


uint8_t nes_controller_read(void)
{
  uint8_t controller_data = 0;
  uint8_t i = 0;
  uint8_t latchtime = 30;//u_sec delay between clock pulses 

  nes_gpio_port &= ~nes_latch_pin; 
  controller_data = !!(nes_input_port & nes_data_pin); //NES yellow wire

  for (i=0; i<7; i++) {
     nes_gpio_port |= nes_clock_pin;
     //_delay_us(latchtime); 
     controller_data = controller_data << 1;
     controller_data = controller_data + !!(nes_input_port & nes_data_pin); 
     nes_gpio_port &= ~nes_clock_pin;
     //_delay_us(latchtime); 
  }
  
  nes_gpio_port |= nes_latch_pin;  

  return controller_data;
}