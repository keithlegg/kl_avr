#ifndef _nesCtrlr_
#define _nesCtrlr_

#include <avr/io.h>

//NES SETUP - THIS SHARES PORTF WITH LCD (LCD IS 2 MSB bits) 
#define nes_gpio_ddr DDRD             
#define nes_gpio_port PORTD  //use bits 3-7  //0b01111100
#define nes_input_port PIND  

#define nes_latch_pin 0x1    // NES Orange wire  //fakeNES yellow
#define nes_clock_pin 0x2    // NES Red wire     //fakeNES blue
#define nes_data_pin 0x4     // NES Yellow       //fakeNES brown 
                                                 //fakeNES power =  5V=RED, GND= WHITE 

void NES_CTRLR_Init( void );


uint8_t nes_controller_read(void);



#endif