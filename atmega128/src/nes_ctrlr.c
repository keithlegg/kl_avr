
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




/*******************/


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
void all_ports_on(void){
   DDRB = 0xff;        
   DDRD = 0xff;        
   DDRF = 0xff;       

   PORTB = 0xff;
   PORTD = 0xff;
   PORTE = 0xff;
   PORTF = 0xff;
}
/*******************/
void blink_port(void){
   blinkddr = 0xff;
   uint8_t delay_ms = 100;
   while (1)
   {
       blinkport |= 0x01;
       _delay_ms(delay_ms);
       blinkport |= 0x02;
       _delay_ms(delay_ms);
       blinkport |= 0x04;
       _delay_ms(delay_ms);
       blinkport |= 0x08;
       _delay_ms(delay_ms);
       blinkport |= 0x10;
       _delay_ms(delay_ms);
       blinkport |= 0x20;
       _delay_ms(delay_ms);
       blinkport |= 0x40;
       _delay_ms(delay_ms);
       blinkport |= 0x80;
       _delay_ms(delay_ms);

       blinkport = 0x00; 
       _delay_ms(delay_ms);       
   }

}
/*******************/

int main (void)
{

   DDRD = 0xff;

   init_nes_ctrl();
   while(1){
       DDRD = controllerRead();//echo nes byte to portD from portB  
   }

   //blink_port();


} 

