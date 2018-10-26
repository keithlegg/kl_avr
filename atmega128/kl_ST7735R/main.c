/*
   Keith Legg Jan 7 - 2016

   //-----------------------------------
   //ORIGINAL ADAFRUIT STUFF:

    3-5V Vin connects to the Arduino 5V pin 
    GND connects to Arduino ground 
    CLK connects to SPI clock. Digital 13. 
        On Mega's, its Digital 52 and on Leonardo/Due its ICSP-3 (See SPI Connections for more details) 
    MOSI connects to SPI MOSI.  Digital 11. 
        On Mega's, its Digital 51 and on Leonardo/Due its ICSP-4 (See SPI Connections for more details) - this is the white wire
    CS connects to our SPI Chip Select pin. 
        We'll be using Digital 10 but you can later change this to any pin  
    RST connects to our TFT reset pin.
        We'll be using Digital 9 but you can later change this pin   
    D/C connects to our SPI data/command select pin. 
        We'll be using Digital 8 but you can later change this pin  

   //-----------------------------------
   //OBSERVATIONS 
      SPI:
         MSB first - Leading edge 
         ~2us per byte speed estimate 

     DC IS LOW FOR CMDS (ST7735_SWRESET, ETC) - FOR "ARGS" IT IS HIGH
     ADAFRUIT CODE CALLS DC  "RS" or "_rs"

   //-----------------------------------
   // HARDWARE WIRING  -PORTB CONFLICTS WITH SPI!!!!
   pin8   | PB0 | D/C
   pin9   | PB1 | NC           - RESET HARDWIRED HIGH 
   pin10  | PB2 | SPI ENABLE   - TFT_CS 
   pin11  | PB3 | SPI MOSI     - WE RUN AS SPI MASTER 
   pin12  | PB4 | NC - MISO    - NOT USED 
   pin13  | PB5 | SPI CLOCK    - 
   +5/+3  |     | RESET 
          |     | NC - CARD_CS 

   //-----------------------------------
   |PORTF | PORTA| 
   |PORTE | PORTC| <---- PORT E CONFLICTS WITH TX/RX
   |-------------| 
   |PORTB |<----- CONFLICTS WITH SPI - BEWARE
   |PORTD |<----- BEWARE
   //-----------------------------------

*/



#include <avr/io.h>
#include <util/delay.h>

//#include "uart.h"
#include "sram.h"
#include "nes_ctrlr.h"
#include "ST7735.h"

#include "kernel_splatr.h"


extern volatile uint8_t lastx;
extern volatile uint8_t lasty;
extern volatile uint8_t x;
extern volatile uint8_t y;
extern volatile uint16_t CRSR_COLOR;


/*******************************/
/*******************************/


/*******************************/
//yay - finally found a good use of pointers in AVR C
uint8_t and_bits(uint8_t *byte, uint8_t byte_a, uint8_t byte_b){
    *byte = 0xff;
    *byte &= byte_a;
    *byte &= byte_b;
}

/*******************************/
//yay - finally found a good use of pointers in AVR C
uint8_t or_bits(uint8_t *byte, uint8_t byte_a, uint8_t byte_b){
    *byte = 0x00;
    *byte |= byte_a;
    *byte |= byte_b;
}



/*******************************/
/*THESE USE GLOBALS INSTEAD OF RETURNING A VALUE!!!*/
/*
void cvt_16_to_8x2(uint16_t input){
   //break 16 bit into 2 bytes - output goes to global buffer
    if(input<256){
        buf1_lsb = input; 
        buf1_msb = 0; 
    }        
    if(input>=256){
        buf1_lsb = (uint8_t)(input & 0x00FF);
        buf1_msb = (uint8_t)(input >> 8);
    }
}

uint16_t cvt_8x2_to_16(uint8_t msbin, uint8_t lsbin){
   uint8_t dat_8;
   uint16_t dat_16;
   dat_8 = msbin;
   dat_16 = (lsbin<<8);
   return (dat_16 | dat_8);
}
*/



/*******************************/

/* DEBUG NO WORKY 
void test_all_ports(volatile uint8_t* port){
    //datadir = 0xff;
    //DDRA = 0xff;

    uint8_t x = 0;
    for (x=0;x<8;x++){
        *port |= (1 << x);      
         _delay_ms(150);
    }     
}


void test_all_ports(){
    uint8_t x = 0;
    for (x=0;x<8;x++){
        PORTA |= (1 << x);      
         _delay_ms(300);
    }     
    PORTA = 0x00;
}

*/

/*******************************/

int main (void)
{


    DDRA = 0xff; //TOP RIGHT PORT 
    DDRC = 0xff; //BOT RIGHT PORT
    DDRG = 0xff; //for SPI/TFT
    DDRB = 0xff; //for SPI
    DDRF = 0xff; //CONTROL BUS
    DDRE = 0x00; //INPUT TEST 

    //SPCR |= (1<<MSTR)|(1<<SPE)|(1<<CPHA)|(1<<CPOL);  // SPI Master, SPI Enable, Trailing edge
    SPCR |= (1<<MSTR)|(1<<SPE);  // SPI Master, SPI Enable
    SPSR |= SPI2X;
   
    ST7735_InitR(INITR_BLACKTAB);  
    ST7735_FillScreen(0x00);
    
    uint8_t nes_byte;

    NES_CTRLR_Init();

    // USART_Init();
    // while(1){
    //     USART_Transmit(0x42);
    //     _delay_ms(100);
    // }


    boot_screen();
    draw_cursor(CRSR_COLOR);
   
    while(1)
    {
        uint8_t nes_byte = nes_controller_read();
        if(nes_byte == 0xFE ) { crsr_right();  } //right
        if(nes_byte == 0xFD ) { crsr_lft();    } //left
        if(nes_byte == 0xFB ) { crsr_dwn();    } //down
        if(nes_byte == 0xF7 ) { crsr_up();     } //up
        if(nes_byte == 0x7F ) { bios_l_click(); } //button a

        if (lastx!=x || lasty!=y){
            ST7735_FillScreen(0x00);
            boot_screen();
            draw_cursor(CRSR_COLOR);
        }
        lastx = x;
        lasty = y;
    }


}//main



