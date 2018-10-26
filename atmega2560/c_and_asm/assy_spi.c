
// assy_spi.c 
#define F_CPU 16000000UL     //16Mhz clock
#include <avr/io.h>
#include <util/delay.h>

//declare the assembly language spi function routine
extern void sw_spi(uint8_t data); 
int main(void)
{
	DDRB = 0x07;       //set port B bit 1,2,3 to all outputs
	while(1){
		sw_spi(0xA5);    //alternating pattern of lights to spi port
		sw_spi(0x5A); 
	} //while 
} //main