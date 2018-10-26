#include <avr/io.h>


#define F_CPU 16000000UL
#define FOSC 16000000UL

#include <util/delay.h>

/*
        MCP23S17 SPI 16-bit IO expander
        Keith Legg, 11/20/16 

    Datasheet:
       http://ww1.microchip.com/downloads/en/DeviceDoc/21952b.pdf

*/


#define SPI_PORT PORTB       //keith was here - SPI for atmega128 is PORTB 
uint8_t _ss          = 0x01; //PB01
uint8_t _address     = 0 ;   //address of 3 digit hardware device  

uint8_t write_cmd    = 0b01000000 | _address<<1 | 0<<0;  // MCP23S17_WRITE = B0100AAA0
uint8_t read_cmd     = 0b01000000 | _address<<1 | 1<<0;  // MCP23S17_READ  = B0100AAA1 


// Config options
const static uint8_t IOCONA = 0x0A;   // Config register is here on power up

// MCP23S17 Registers Definition for BANK=0 (default)
// "TABLE 3-5:  CONTROL REGISTER SUMMARY (IOCON.BANK = 0) page 17"
const static uint8_t IODIRA = 0x00; //DDR PORTA
const static uint8_t IODIRB = 0x01; //DDR PORTB
const static uint8_t GPPUA  = 0x0C;
const static uint8_t GPPUB  = 0x0D;
const static uint8_t GPIOA  = 0x12;
const static uint8_t GPIOB  = 0x13;

//-------------------------------------
uint8_t uint16_high_byte(uint16_t uint16)
{
    return (uint8_t)(uint16>>8);
}

//-------------------------------------
uint8_t uint16_low_byte(uint16_t uint16)
{
    return (uint8_t)(uint16 & 0x00FF);
}

//-------------------------------------
void spi_transmit(uint8_t byte){
    //we intentionally dont use CS line here in case of sending multi byte messages
    SPDR = byte;                 // send the data
    while(!(SPSR & (1<<SPIF)));  // wait until transmission is complete
}

//-------------------------------------
uint8_t spi_recieve(uint8_t byte){
    //this is the same function as transmit, except wait AFTER setting byte and know that the input is going to get "lost"  
    //we intentionally dont use CS line here in case of sending multi byte messages    
    SPDR = byte;                 // send the data
    while(!(SPSR & (1<<SPIF)));  // wait until transmission is complete
    return SPDR;         
}

//-------------------------------------
void write_addr(uint8_t reg, uint16_t word) {  // Accept the start register and word 
    SPI_PORT &=~ _ss;                                    // Take slave-select low
    spi_transmit(write_cmd);                             // Send the MCP23S17 opcode, chip address, and write bit
    spi_transmit(reg);                                   // Send the register we want to write 
    spi_transmit((uint8_t) (word));                      // Send the low byte (register address pointer will auto-increment after write)
    spi_transmit((uint8_t) (word >> 8));                 // Shift the high byte down to the low byte location and send
    SPI_PORT |= _ss;                                     // Take slave-select high
}


//-------------------------------------
uint8_t read_addr(uint8_t reg) {                         // Accept the start register and word 
    uint8_t value = 0;                                   // Initialize a variable to hold the read values to be returned
    SPI_PORT &=~ _ss;                                    // Take slave-select low
    spi_transmit(read_cmd);                              // Send the MCP23S17 opcode, chip address, and write bit
    spi_transmit(reg);                                   // Send the register we want to read 
    value = spi_recieve(0x00);                           // Send any byte, the function will return the read value
    SPI_PORT |= _ss;                                     // Take slave-select high
    return value;
}

//-------------------------------------
void setup_spi(){
    DDRB = 0x07;        //CS/SlaveSelect, MOSI, and CLK  as an output

    SPI_PORT |= _ss;    //set CS line high, or off (active low)
    SPCR |= (1<<MSTR);  // Set AVR SPI as Master
    SPCR |= (1<<SPE);   // Enable SPI hardware

    //begin by pulsing CS and leave high (inactive)
    SPI_PORT |= _ss;                          
    _delay_ms(1);    
    SPI_PORT &=~ _ss;                          
    _delay_ms(1);       
    SPI_PORT |= _ss;                                 
}

//-------------------------------------

int main (void)
{

    DDRF = 0xff; //TO SHOW A RESULT BYTE RETURNED FROM MCP23S17

    setup_spi();

    // Initialize MCP23S17 
    write_addr(IOCONA,0x28);   // I/O Control Register: BANK=0, SEQOP=1, HAEN=1 (Enable Addressing)
    write_addr(IODIRA,0x00);   // GPIOA As Output
    write_addr(GPIOA,0x00);    // Reset Output on GPIOA   

    //SET MCP23S17 PORTB AS INPUT  
    write_addr(IODIRB,0xFF);   // GPIOB As Input
    //write_addr(GPPUB,0xFF);    // Enable Pull-up Resistor on GPIOB

    //TO SET MCP23S17 PORTB as OUTPUT
    //write_addr(IODIRB,0x00);   // GPIOB As Output
    //write_addr(GPIOB,0x00);    // Reset Output on GPIOB 

    uint8_t delay_ms = 200;
    uint8_t i = 0;
    
    while(1){

        //TEST CODE FOR OUTPUT - FLIP THE BITS ON THE OUTPUT
        // for(i=0;i<8;i++){
        //    write_addr(GPIOB, (1 << i));
        //    _delay_ms(delay_ms);         
        // }
        // write_addr(GPIOB, 0x00);

        //TEST CODE FOR INPUT - SHOW THE RESULT BYTE ON PORTF
        PORTF = read_addr(GPIOB);
        _delay_ms(100);
    }

}


