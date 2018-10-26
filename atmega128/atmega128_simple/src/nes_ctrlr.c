
#include <avr/io.h>

#define F_CPU 16000000UL //AVR Clock Speed in MHZ
#define FOSC 16000000    // Clock Speed


#define blinkport PORTD
#define blinkddr DDRD

#define nesctrl_port PORTB
#define nesctrl_ddr DDRB
#define nes_data_pin 0x1  // NES Yellow  
#define nes_latch_pin 0x2 // NES Orange wire
#define nes_clock_pin 0x4 // NES Red wire
                          // BROWN - PWR_FLAG
                          // WHITE - PWR_FLAG

#include <util/delay.h>



//   NES CONTROLLER WIRING 
//   white   - +5
//   red     - clock
//   orange  - latch
//   yellow  - data 
//   brown   - ground   
//---------------------
//PF---PA
//PE---PC
//---------------------
//PB---|
//PD---|
//---------------------



void init_nes_ctrl(void){
    nesctrl_ddr = 0x00;
}

/*******************/

uint8_t controllerRead(void){
    uint8_t controller_data = 0;
    uint8_t i = 0;
    uint8_t latchtime = 30;//u_sec delay between clock pulses 
   
    nesctrl_port &=~ nes_latch_pin; 
    controller_data = !!(PINB & nes_data_pin); //NES yellow wire

    for (i=0; i<7; i++) {
       nesctrl_port |= nes_clock_pin;
       _delay_us(latchtime); 
       controller_data = controller_data << 1;
       controller_data = controller_data + !!(PINB & nes_data_pin); 
       nesctrl_port &=~ nes_clock_pin;
       _delay_us(latchtime); 
    }
  
    nesctrl_port |= nes_latch_pin;  

    return controller_data;
}

/*******************/

int main (void)
{

   DDRD = 0xff;

   init_nes_ctrl();
   while(1){
       DDRD = controllerRead();//echo nes byte to portD from portB  
   }

} 
