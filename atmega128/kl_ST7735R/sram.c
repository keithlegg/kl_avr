

#include <avr/io.h>

#include "sram.h"
#include "uart.h"

//FROM KERNEL SPLATTER 
//standard buffers for 8 to 16 bit stitching and unwrapping
//often we just need 3 bytes (2 addr+1 data) in that case ignore com_msb2

uint8_t buf1_lsb = 0;
uint8_t buf1_msb = 0;



/*******************************/

/*

    (17 bit address bus) I wired 17 high to test 
    --------------------------------
    address (L)  (H)    | CONTROL 
           PORTA PORTC  | PORTF
           [0-7] [8-13] | [1-3]  
    --------------------------------
    PORTA - address bus low   (7 bit)
    PORTC - address bus high  (7 bit)
    PORTE - data bus  - CONFLICTS WITH SERIAL0!!!!!
    PORTF - control bus       (3-4 bits)

   ----------------CY7C109BN-------------------
                _____   _____
              NC|1   |_|   32|Vcc
             A16|2         31|A15
             A14|3         30|CE2 - WIRE TO VCC!
             A12|4         29|/WE  -active low   
              A7|5         28|A13  
              A6|6         27|A8 
              A5|7         26|A9
              A4|8         25|A11
              A3|9         24|/OE  -active low 
              A2|10        23|A10
              A1|11        22|/CE1 - active low 
              A0|12        21|IO8
             IO1|13        20|IO7
             IO2|14        19|IO6
             IO3|15        18|IO5
             GND|16        17|IO4
                -------------
*/


/***********************************************/

#define WE_PIN 0x01 
#define OE_PIN 0x02
#define CS_PIN 0x04  //CE is CS 

#define sram_loaddr PORTA    
#define sram_hiaddr PORTC    
#define sram_ctrlport PORTF  



/***********************************************/
void clear_sram(void){
   //clear entire memory space 
   flood_sram(0x00);
}

/***********************************************/
void flood_sram(uint8_t byte){
   uint16_t a = 0;   
   for(a=0;a<65535;a++){ 
      write_ram(a, byte ); 
   } 
}

/***********************************************/

/*
  fill SRAM with a distinctive pattern to test

  a%256/2 
  = 
  00000000
  00000000
  00000001
  00000001
  00000010
  00000010
  ... etc
*/

void flood_pattern(void){
   uint16_t a = 0;
   uint8_t ct =0; 
   for(a=0;a<10;a++){ 
      write_ram(a, (uint8_t)(a%256/2 & 0x00FF) ); //DONT WRITE 16 bit value !
   } 

}

/***********************************************/
/*
   send a list of text based byte representations of SRAM locations over serial port
   same as tx_ram except you can start counting at any address
*/

void show_ram_region(uint16_t startaddr, uint16_t endaddr){
   uint16_t a = startaddr;
   if(endaddr>startaddr){
       for(a=startaddr;a<endaddr;a++){
          USART_Transmit(0x61); // a
          USART_Transmit(0x20); //space
          send_txt_byte( (uint8_t)(a >> 8) , 0);//LSB of address
          send_txt_byte( (uint8_t)(a & 0x00FF), 0);//MSB of address
          USART_Transmit(0x20); //space      
          USART_Transmit(0x64); // d
          USART_Transmit(0x20); //space
          send_txt_byte( read_ram(a), 1);//data
       }
   }

}


/***********************************************/
void tx_bytes(uint16_t startaddr, uint16_t numbytes)
{
     uint16_t b = 0;

     for(b=startaddr;b<numbytes;b++)
     {
         uint8_t tmp = read_ram(b);
         while ( !( UCSR0A & (1<<UDRE0)) );
         UDR0 = tmp;
        
     }//B-Y
}

/***********************************************/
/*
  serial port bootloader. Slow, clunky, and I love it!
  fill local SRAM with data sent over uart from host machine 
*/

void rx_ram(uint16_t numbytes){
   uint16_t a = 0;

   //clamp at 65535
   for(a=0;a<numbytes;a++)
   { 
       write_ram(a, USART_receive() ); //pack 16 bit value into 1 byte to send 
   }
}


/***********************************************/
uint8_t set_address(uint16_t address){
    uint8_t low_byte  = 0;
    uint8_t high_byte = 0;
   
    if(address<256){
        high_byte = 0; 
        low_byte  = address; 
    }        
    else{
        //high byte, shift all bits 8 places right
        high_byte = (uint8_t)(address >> 8);
        //low byte, clear the high byte
        low_byte = (uint8_t)(address & 0x00FF);
    }
    
    sram_loaddr = low_byte;
    sram_hiaddr = high_byte;
}



/***********************************************/
uint8_t read_ram(uint16_t address){
    
    uint8_t out = 0x00;

    DDRE = 0x00; 
    PORTF |= CS_PIN; //active low - dont think we need this?
    PORTE = 0x00; //clear data port
    set_address(address);

    PORTF |= WE_PIN; //WE high when reading 
    PORTF |= OE_PIN; 

    PORTF &= ~CS_PIN; 
    PORTF &= ~OE_PIN; 

    out = PINE;
    PORTE = 0x00; //clear data port

    PORTF |= OE_PIN; 
    PORTF |= CS_PIN; 

    return out;
}

/***********************************************/

void write_ram(uint16_t address, uint8_t byte){
    
    DDRE = 0xff;
    PORTF |= CS_PIN;  
    PORTF |= WE_PIN; 
    PORTF |= OE_PIN;     

    set_address(address);

    PORTF &= ~CS_PIN;  

    PORTE = byte;

    PORTF &= ~WE_PIN;     
    PORTF |= WE_PIN;  

}


/*******************************/
/*THESE USE GLOBALS INSTEAD OF RETURNING A VALUE!!!*/
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

/*******************************/
uint16_t cvt_8x2_to_16(uint8_t msbin, uint8_t lsbin){
   uint8_t dat_8;
   uint16_t dat_16;
   dat_8 = msbin;
   dat_16 = (lsbin<<8);
   return (dat_16 | dat_8);
}


/** 
void sram_erase(void){
    unsigned char foo[] = "Erasing SRAM";
    ST7735_DrawString(0,90, foo );
    //------
    uint16_t a = 0; 
    for(a=0;a<65535;a++){
        write_ram(a, 0x00);
        PORTE = 0x00;//stupid hack - debug
        //render_16_hex(0  , 100 , 2  , a      , CRSR_COLOR);  //cool but too slow         
    }
}
*/
