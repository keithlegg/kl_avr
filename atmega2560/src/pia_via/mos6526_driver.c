#include <avr/io.h>

#define F_CPU 16000000UL //AVR Clock Speed in MHZ
#define FOSC 16000000    // Clock Speed

//UART STUFF
#define BAUD 57600
#define MYUBRR FOSC/16/BAUD-1

#define BIT_ON 0x30 //logic high
#define BIT_OFF 0x31 //logic low

#include <util/delay.h>

/***********************************************/

/***********************************************/

/*
  
   THIS CODE WAS WRITTEN FOR A 6522 - BUT IT WORKS WITH A 6526 AS WELL 
   SAME WIRING AND ALL!!!



  /////////////////////////
  SUMMARY 

  PA0 -   PA7   I/O-Port A Bit0-7 Bidirectional parallell port
  PB0 -   PB7   I/O-Port B Bit0-7 Bidirectional parallell port
  /PC     Port Control - Handshake output, Low pulse after read/write on port B.
  TOD     Time Of Day clock input, either 50Hz or 60Hz input.
  /IRQ    Interrupt Request, output to CPU's IRQ input
  R/W     High-level = Read (Move data from CIA), Low-level = Write (Move data to CIA)
  /CS     Chip Select - low level means active CIA.
  /FLAG   Negative edge IRQ input, can be used as handshake for either parallel port.
  /phi2   Processor Φ2 clock, data transfers occur only on high signal level.
  DB0 -   DB7   Bidirectional data bus Bits 0 - 7
  /RES    Reset input, low signal initiates CIA.
  RS0 -   RS3   Register Select - Used to interface internal registers with bidirectional ports.
  SP      Serial Port - bidirectional, internal shift register converts CPU parallel data into serial data and vice-versa.
  CNT     Count - Internal timers can count pulses to this input. Can be used for frequency dependent operations. 

  /////////////////////////

  6526 INTERFACE SIGNALS

  02 — Clock Input
  The 02 clock is a TTL compatible input used for
  internal device operation and as a timing reference for
  communicating with the system data bus.

  RS3-RS0 — Address Inputs
  The address inputs select the internal registers as
  described by the Register Ma

  DB7-BD0 — Data Bus Inputs/Outputs
  The eight data bus pins transfer information between
  the 6526 and the system data bus. These pins are high
  impedance inputs unless CS is low and R/W and 02 are
  high, to read the device. During this read, the data bus
  output buffers are enabled, driving the data from the
  selected register onto the system data bus.

  CS — Chip Select Input
  The CS jnput controls the activity of the 6526. 
  A low level on CS while 02 is high causes the device to
  respond to signals on the R/W and address (RS) lines. 

  A high on CS_prevents these lines from controlling the
  6526. The CS line is normally activated (low) at PHI2 by the
  appropriate address combination.

  IRQ — Interrupt Request Output
  IRQ is an open drain output normally connected to
  the processor interrupt input. An external pullup resistor
  holds the signal high, allowing multiple IRQ outputs to
  be connected together. The IRQ output is normally off
  (high impedance) and is activated low as indicated in
  the functional description.

  R/W — Read/Write Input
  The R/W signal is normally supplied by the micro­
  processor and controls the direction of data transfers of
  the 6526. A high on R/W indicates a read (data transfer
  out of the 6526), while a low indicates a write (data
  transfer into the 6526).

  RES — Reset Input
  A low on the RES pin resets all internal registers.The
  port pins are set as inputs and port registers to zero
  (although a read of the ports will return all highs
  because of passive pullups).The timer control registers
  are set to zero and the timer latches to all ones. All other
  registers are reset to zero

*/

/***********************************************/

/*

                  MOS6526
               _____   _____
           VSS |1   |_|   40| CNT
           PA0 |2         39| SP
           PA1 |3         38| RS0
           PA2 |4         37| RS1 
           PA3 |5         36| RS2
           PA4 |6         35| RS3
           PA5 |7         34| /RESET - RSTPIN (active low)
           PA6 |8         33| DB0
           PA7 |9         32| DB1
           PB0 |10        31| DB2
           PB1 |11        30| DB3
           PB2 |12        29| DB4
           PB3 |13        28| DB5
           PB4 |14        27| DB6
           PB5 |15        26| DB7
           PB6 |16        25| PHI2  - CLKPIN   
           PB7 |17        24| /FLAG (active low)
active low /PC |18        23| /CS  - CSPIN (active low)   
(time day) TOD |19        22| R/W  - RWPIN
           VCC |20        21| /IRQ - (active low)
               --------------  




*/

/***********************************************/

/*
    # WIRING SETUP (all done with 2 ports)

  |Vcc   |Vcc  |      PORTK      |  PC[0-4]          | PC5   |  PC6 | PC7 
  |Reset |CS1  | 8 bit data bus  |  Register Select  | CS2   |  R/W | phi2 Clock
  _____________________________________________

   #PIN COUNT 

   data bus            X8
   register select     X4
   CS1 and CS2         X1   -  Active low on 2, 1 wired HIGH permanente
   R/W                 X1
   CLK (enable)        X1
  _____________________________________________

*/


/***********************************************/
/***********************************************/
                   
#define CLKPIN 0x80 //PC7 //clock can be thought of as enable
#define CS2PIN 0x20 //PC5 
#define RWPIN 0x40  //PC6




/***********************************************/
/***********************************************/

void USART_Init( unsigned int ubrr)
{
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);

}


/***********************************************/

static uint8_t USART_receive(void)
{
    while (!(UCSR0A & (1 << RXC0))) {}
    return UDR0;
}

/***********************************************/

void USART_Transmit( unsigned char data )
{
  while ( !( UCSR0A & (1<<UDRE0)) );
  UDR0 = data;
}


/***********************************************/

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
    USART_Transmit( 0xa ); //CHAR_TERM = new line  
    USART_Transmit( 0xd ); //0xd = carriage return
}

/***********************************************/
uint8_t idx_to_byte(uint8_t idx){
    return (1 << idx);
} 


/***********************************************/

/***********************************************/

/*
  6526 
  
  _________________
  3 2 1 0      REG
  _________________
  0 0 0 0 | 0  PRA      PERIPHERAL DATA REG A
  0 0 0 1 | 1  PRB      PERIPHERAL DATA REG B
  0 0 1 0 | 2  DDRA     DATA DIRECTION REG A
  0 0 1 1 | 3  DDRB     DATA DIRECTION REG B
  0 1 0 0 | 4  TA LO    TIMER A LOW REGISTER
  0 1 0 1 | 5  TA HI    TIMER A HIGH REGISTER
  0 1 1 0 | 6  TB LO    TIMER B LOW REGISTER
  0 1 1 1 | 7  TB HI    TIMER B HIGH REGISTER
  1 0 0 0 | 8  TOD 10TH 10THS OF SECONDS REGISTER
  1 0 0 1 | 9  TOD SEC  SECONDS REGISTER
  1 0 1 0 | A  TOD MIN  MINUTES REGISTER
  1 0 1 1 | B  TOD HR   HOURS — AM/PM REGISTER
  1 1 0 0 | c  SDR      SERIAL DATA REGISTER
  1 1 0 1 | D  ICR      INTERRUPT CONTROL REGISTER
  1 1 1 0 | E  CRA      CONTROL REG A
  1 1 1 1 | F  CRB      CONTROL REG B

*/

void set_register_select(uint8_t byte){
    PORTC = byte&0x0f; //use lower 4 bits of argument for register mapping
}


/***********************************************/

void pulseClock(void){
    PORTC |= CLKPIN;
    PORTC &=~ CLKPIN;
}

/***********************************************/

/*
    To output to a peripheral from the CPU through the I/O bus:

    1. Set the RS (register select) pins to 0010 or 0011 (for DDRA or DDRB).
    2. Set the data bus to 11111111. (All 1s, making all the I/O bus pins
      outputs.)
    3. Enable both CSes.
    4. Make sure R/W is low.
    5. Pulse CLK2.
    (DDRA or DDRB is now set to make all I/O pins on that bus act as outputs.)
    /////////////////
    6. Set the RS pins to 0001 or 0000 (for ORA or ORB).
    7. Set the data bus to whatever you want to output on the I/O bus.
    8. Enable both CSes.
    9. Make sure R/W is low.
    10. Pulse CLK2.
*/

void output_port(uint8_t outbyte){

    //set Data Direction 
    set_register_select(0x02); //0010 = PORTA, 0011 = PORTB 
    DDRK = 0xff; 
    PORTK = 0xff; //set data port all high
    PORTC &=~CS2PIN; //set CS2 Low (enabled, active low) 
    PORTC &=~RWPIN; //set R/W low 
    pulseClock();

    /****/
    //Send byte out to respective port 
    set_register_select(0x00); //0000 = PORTA, 0001  = PORTB
    PORTK=outbyte; //byte going out the pipe 
    PORTC &=~CS2PIN; //set CS2 Low (enabled, active low) 
    PORTC &=~RWPIN; //set R/W low 
    pulseClock();
}

/***********************************************/

/*
    MAKE SURE YOU DONT LEAVE ANYTHING ON DATA BUS FLOATING!!!!
    OTHERWISE YOU CANT TRUST WHAT YOU GET BACK.
    IT SEEMS TO PULL BITS HIGH IF LEFT OPEN.

    To input from a peripheral to the CPU through the I/O bus:

    1. Set the RS pins to 0010 or 0011 (for DDRA or DDRB).
    2. Set the data bus to 00000000. (All 0s, making all the I/O bus pins
      inputs.)
    3. Enable both CSes.
    4. Make sure R/W is low.
    5. Pulse CLK2.
    (DDRA or DDRB is now set to make all I/O pins on that bus act as inputs.)

    6. Set the RS pins to 0000 or 0001 (for IRA or IRB).
    7. Enable both CSes.
    8. Make sure R/W is high.
    9. Pulse CLK2.
    (Data from the selected I/O bus should now appear on the system data bus.)
*/


uint8_t input_port(void){

    //set Data Direction 
    set_register_select(0x03); //0011 = PORTA, 0010 = PORTB 
    DDRK = 0xff;  //DDR OUT to set data bus low
    PORTK = 0x00; //set data port all LOW
    PORTC &=~CS2PIN; //set CS2 Low (enabled, active low) 
    PORTC &=~RWPIN; //set R/W LOW - RWB must always be preceded by CS1, CS2B 
    pulseClock();

    /****/
    DDRK = 0x00;  //now we are done setting DDR - trisate the data bus

    //Send byte out to respective port 
    set_register_select(0x01); //0001 = PORTA, 0000 = PORTB
    PORTC &=~CS2PIN; //set CS2 Low (enabled, active low) 
    PORTC |= RWPIN;  //set R/W high - RWB must always be preceded by CS1, CS2B 
    pulseClock();
    return PINK;

}


/***********************************************/
void run_in(){
    uint8_t mybyte = input_port();
    print_byte(mybyte);
    _delay_ms(600);      
}

/***********************************************/
void run_out(){
    int del = 100;

    output_port(0x01);
    _delay_ms(del);

    output_port(0x02);
    _delay_ms(del);

    output_port(0x04);
    _delay_ms(del);
                   
    output_port(0x08);
    _delay_ms(del);

    output_port(0x10);
    _delay_ms(del);

    output_port(0x20);
    _delay_ms(del);
 
    output_port(0x40);
    _delay_ms(del);

    output_port(0x80);
    _delay_ms(del);                                         
}


int main (void)
{
   DDRC = 0xff;     
   DDRL = 0xff; 

   USART_Init(MYUBRR);

   PORTC &=~ CLKPIN;

   // if you want to run through a range of addresses   
   while(1){
      run_in();
      //run_out();

   }
    

} 

/***********************************************/

