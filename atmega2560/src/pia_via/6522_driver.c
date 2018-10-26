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

/*

// http://quinndunki.com/blondihacks/?p=1347

// Some generally useful macros
#define QDDRB(n) (1<<(DDB##n))
#define QPINBIT(n) PB##n
#define READPIN(pin) ((PINB & (1<<(QPINBIT(pin)))) > 0)
#define SET_HI(pin) PORTB |= (1<<(QPINBIT(pin)))
#define SET_LO(pin) PORTB &= ~(1<<(QPINBIT(pin)))
#define PULSE(pin) SET_HI(pin); SET_LO(pin);



                      MOS6522
                   _____   _____
               VSS |1   |_|   40| CA1    NC
               PA0 |2         39| CA2    NC
               PA1 |3         38| RS0    (Register Select 0)
               PA2 |4         37| RS1    (Register Select 1)
               PA3 |5         36| RS2    (Register Select 2)
               PA4 |6         35| RS3    (Register Select 3)
               PA5 |7         34| /RESET - active low  - (keep high)
               PA6 |8         33| DB0
               PA7 |9         32| DB1
               PB0 |10        31| DB2
               PB1 |11        30| DB3
               PB2 |12        29| DB4
               PB3 |13        28| DB5
               PB4 |14        27| DB6
               PB5 |15        26| DB7
               PB6 |16        25| PHI2   
               PB7 |17        24|  CS1  - (Chip Select 1; Active-high) (keep high)
        NC     CB1 |18        23| /CS2  - (Chip Select 2; Active-low)  
        NC     CB2 |19        22| R/W   - (High = read, low = write)
               VCC |20        21| /IRQ  - active low 
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


/*

-------------------
//http://forum.6502.org/viewtopic.php?t=342#p2310

The 6522 has a bug in mode 011, shifting synchronous serial data in under control of external CB1 clock. 
If the edge on CB1 falls within a few nanoseconds of the falling edge of phase 2, the CB1 edge will be ignored, 
so you lose a bit. Worse, you'll get framing errors on subsequent data. To get around it, put the external 
clock signal into the D input of a 74HC74 flip-flop, run the flip-flop's Q output to the 6522's CB1 pin, 
and clock the flip-flop with phase 0 or phase 2. I have used this to send data from one 6502 computer 
to another using CA2 for an extra handshake line.


-------------------


RESET ()

A low reset () input clears all 6522 internal registers to logic 0 (except T1 and T2) latches and 
counters and the Shift Register). This places all peripheral interface lines in the input state,
 disables the timers, shift register, etc. and disables interrupting from the chip.

-------------------
INPUT CLOCK ()
The input clock is the system clock and triggers all data transfers between processor bus and the 6522.

-------------------
READ/WRITE ()
The direction of the data transfers between the R6522 and the system processor is controlled by the
 line in conjunction with the CS1 and inputs. When low (write operation), and the R6522 is selected,
  data is transferred from the processor bus into the selected R6522 register. When is high 
  (read operation), and the R6522 is selected, data is transferred from the selected R6522 register 
  to the processor bus.

-------------------
DATA BUS (D0-D7)

The eight bidirectional data bus lines transfer between the R6522 and the system processor bus. During 
read cycles, the contents of the selected R6522 register are placed on the data bus lines. During write 
cycles, these lines are high-impedance inputs and data is transferred from the processor bus into the 
selected register. When the R6522 is not selected, the data bus lines are high impedance.

-------------------
CHIP SELECTS (CS1, CS2)

The two chip select inputs are normally connected to processor address lines either directly or through 
decoding. The selected R6522 register is accessed when CS1 is high and CS2 is low.

-------------------
INTERRUPT REQUEST ()

The Interrupt Request output goes low whenever an internal Interrupt flag is set and the corresponding 
interrupt enable bit is a logic 1. This output is open-drain to allow the interrupt request signal to 
be wire-OR'ed with other equivalent signals in the system.

-------------------
PERIPHERAL PORT A (PA0-PA7)
Port A consists of eight lines which can be individually programmed to act as inputs or outputs under 
control of a Data Direction Register. The polarity of output pins is controlled by an Output Register
 and input data may be latched into an internal register under control of the CA1 line. All of these 
 modes of operation are controlled by the system processor through the internal control registers. These
  lines represents one standard TTL load in the input mode and will drive one standard TTL load in 
  the output mode. Figure 2 illustrates the output circuit.

-------------------
PORT A CONTROL LINES (CA1,CA2)

The two Port A control lines act as interrupt inputs or as handshake outputs. Each line controls an internal 
interrupt flag with a corresponding interrupt enable bit. In addition, CA1 controls the latching of data on 
Port A input lines. CA1 is a high-impedance input only while CA2 represents one standard TTL load in the input
 mode. CA2 will drive one standard TTL load in the output mode.

-------------------

PORT B (PB0-PB7)
Peripheral Port B consists of eight bi-directional lines which are controlled by an output register and a data 
direction register in much the same manner as the Port A. In addition, the polarity of the PB7 output signal 
can be controlled by one of the interval timers while the second timer can be programmed to count pulses on 
the PB6 pin. Port B lines represent one standard TTL load in the input mode and will drive one standard TTL 
load in the output mode. In addtition, they are capable of sourcing 1.0 mA at 1.5 Vdc in the output mode to 
allow the outputs to directly drive Darlington transistor circuits. Figure 3 is the circuit schematic.

-------------------
PORT B CONTROL LINES (CB1,CB2)

The Port B control lines act as interrupt inputs as handshake outputs. As with CA1 and CA2, each line controls
 an interrupt flag with a corresponding interrupt enable bit. In addition, these lines act as a serial port 
 under control of the Shift Register. These lines represent one standard TTL load in the input mode and will
  drive one standard TTL load in the output mode. CB2 can also drive a Darlington transistor circuit; however,
   CB1 cannot.

-------------------

The two CS (Chip Select) pins enable the chip. For the chip to be enabled,
CS1 must be high and CS2 must be low. If these two conditions are not met,
the chip will not respond. These pins are normally tied to the computer's
address bus.

-------------------

CLK2 is normally tied to the 6502 CPU's CLK2 output. This signal will
actually trigger the 6522, and make it perform whatever read/write operation
is selected. Although it is normally pulsed at a constant frequency by the
CPU, you can actually trigger CLK2 by hand, using a push-button switch, if
you are operating the 6522 manually. This may be thought of as the "Enable"
signal, and it is usually called the "Enable" pin in this document, since
that's a little more appropriate a term for it when you're working with the
6522 alone (as opposed to when you're using it in conjunction with the 6502,
where it makes a little more sense since CLK2 or PHI2 refers to a clock
output pin on the 6502).


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

/*
    +---------+---------------+---------+---------------------------------------------+
    |Reg.     |   RS Coding   |Register |          Register/Description               |
    | #       +---+---+---+---+ Desig.  +----------------------+----------------------+
    |         |RS3|RS2|RS1|RS0|         | Write (R/W = L)      | Read (R/W = H)       |
    +---------+---+---+---+---+---------+----------------------+----------------------+
    |  0      | 0 | 0 | 0 | 0 | ORB/IRB | Output Register B    | Input Register B     |
    +---------+---+---+---+---+---------+----------------------+----------------------+
    |  1      | 0 | 0 | 0 | 1 | ORA/IRA | Output Register A    | Input Register A     |
    +---------+---+---+---+---+---------+----------------------+----------------------+
    |  2      | 0 | 0 | 1 | 0 |  DDRB   | Data Direction Register B                   |
    +---------+---+---+---+---+---------+---------------------------------------------+
    |  3      | 0 | 0 | 1 | 1 |  DDRA   | Data Direction Register A                   |
    +---------+---+---+---+---+---------+----------------------+----------------------+
    |  4      | 0 | 1 | 0 | 0 |  T1C-L  | T1 Low-Order Latches | T1 Low-Order Counter |
    +---------+---+---+---+---+---------+----------------------+----------------------+
    |  5      | 0 | 1 | 0 | 1 |  T1C-H  | T1 High-Order Counter| T1 High-Order Counter|
    +---------+---+---+---+---+---------+---------------------------------------------+
    |  6      | 0 | 1 | 1 | 0 |  T1L-L  | T1 Low-Order Latches                        |
    +---------+---+---+---+---+---------+---------------------------------------------+
    |  7      | 0 | 1 | 1 | 1 |  T1L-H  | T1 High-Order Latches                       |
    +---------+---+---+---+---+---------+----------------------+----------------------+
    |  8      | 1 | 0 | 0 | 0 |  T2C-L  | T2 Low-Order Latches | T2 Low-Order Counter |
    +---------+---+---+---+---+---------+----------------------+----------------------+
    |  9      | 1 | 0 | 0 | 1 |  T2C-H  | T2 High-Order Counter                       |
    +---------+---+---+---+---+---------+---------------------------------------------+
    | 10      | 1 | 0 | 1 | 0 |  SR     | Shift Register                              |
    +---------+---+---+---+---+---------+---------------------------------------------+
    | 11      | 1 | 0 | 1 | 1 |  ACR    | Auxiliary Control Register                  |
    +---------+---+---+---+---+---------+---------------------------------------------+
    | 12      | 1 | 1 | 0 | 0 |  PCR    | Peripheral Control Register                 |
    +---------+---+---+---+---+---------+---------------------------------------------+
    | 13      | 1 | 1 | 0 | 1 |  IFR    | Interrupt Flag Register                     |
    +---------+---+---+---+---+---------+---------------------------------------------+
    | 14      | 1 | 1 | 1 | 0 |  IER    | Interrupt Enable Register                   |
    +---------+---+---+---+---+---------+----------------------+----------------------+
    | 15      | 1 | 1 | 1 | 1 | ORA/IRA | Output Register A*   | Input Register A*    |
    +---------+---+---+---+---+---------+----------------------+----------------------+
    | NOTE: * Same as Register 1 except no handshake.                                 |
    +---------------------------------------------------------------------------------+
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
    set_register_select(0x03); //0011 = PORTA, 0010 = PORTB 
    DDRK = 0xff; 
    PORTK = 0xff; //set data port all high
    PORTC &=~CS2PIN; //set CS2 Low (enabled, active low) 
    PORTC &=~RWPIN; //set R/W low 
    pulseClock();

    /****/
    //Send byte out to respective port 
    set_register_select(0x01); //0001 = PORTA, 0000 = PORTB
    PORTK=outbyte; //byte going out the pipe 
    PORTC &=~CS2PIN; //set CS2 Low (enabled, active low) 
    PORTC &=~RWPIN; //set R/W low 
    pulseClock();
}

/***********************************************/

/*
    MAKE SURE YOU DONT LEAVE ANYTHING ON DATA BUS FLOATING!!!!
    OTHERWISE YOU CANT TRUST WHAT YOU GET BACK.
    IT SEEMS TO PULL BITS HIGH IF LEFT OPEN. ( open collector with internal pullups ??? )

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

