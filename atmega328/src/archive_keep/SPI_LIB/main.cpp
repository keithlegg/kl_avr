#include <avr/io.h>


#define F_CPU 16000000UL
#define FOSC 16000000UL

#include <util/delay.h>

#include "SPI.h"
#include "SPI.cpp"


/*

 #include "DAC_MCP49xx.h"

 DAC_MCP49xx dac(DAC_MCP49xx::MCP4912, SS_PIN, LDAC_PIN);
 dac.setSPIDivider(SPI_CLOCK_DIV16);
 dac.setPortWrite(true);
 dac.setAutomaticallyLatchDual(true);

 dac.outputA(xval);
 dac.outputB(yval);
 dac.latch(); 

*/

//  SPI.setBitOrder(MSBFIRST);
//  SPI.setDataMode(SPI_MODE0);
//  SPI.setClockDivider(spi_divider);
//    char data  = 0x04;//0b00001000;

int main (void)
{

  SPI.begin();

  while(1){
	  SPI.transfer(0b00000000);
	  _delay_ms(100);
	  SPI.transfer(0b00001000);
	  _delay_ms(100);
	  SPI.transfer(0b00000101);
	  _delay_ms(100);
  }

}


