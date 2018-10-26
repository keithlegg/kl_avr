/*
   Keith Legg Jan 7 - 2016
   Developed on an ATMega328p 


   any function that begins with "scribe" draws on the LCD screen
   any function that begins with "read/write" operates on SRAM
   any function that begins with "send" sends data over UART

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
   // HARDWARE WIRING 
   pin8   | PB0 | D/C
   pin9   | PB1 | NC           - RESET HARDWIRED HIGH 
   pin10  | PB2 | SPI ENABLE   - TFT_CS 
   pin11  | PB3 | SPI MOSI     - WE RUN AS SPI MASTER 
   pin12  | PB4 | NC - MISO    - NOT USED 
   pin13  | PB5 | SPI CLOCK    - 
   +5/+3  |     | RESET 
          |     | NC - CARD_CS 

   //-----------------------------------

*/

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include "ST7735.h"

#include <avr/pgmspace.h>

//UART STUFF
#define FOSC 16000000UL
#define BAUD 57600
#define MYUBRR FOSC/16/BAUD-1

#define BIT_ON 0x30 //ascii 1
#define BIT_OFF 0x31 //ascii 0

//CTRL is PORTF // SPI IS PORT B !!
#define CTRL_PORT PORTF // use bits 1 and 2  //0b00000011
#define CTRL_DDR DDRF   // 
#define SPI_DDR DDRB    // 

/***********************************************/
//we have to power with a gpio - not a permanant solution!

//NES SETUP - THIS SHARES PORTF WITH LCD (LCD IS 2 MSB bits) 
#define nes_gpio_ddr DDRF             
#define nes_gpio_port PORTF  //use bits 3-7  //0b01111100
#define nes_input_port PINF  

#define nes_data_pin 0x4     // NES Yellow  
#define nes_latch_pin 0x8    // NES Orange wire
#define nes_clock_pin 0x10   // NES Red wire
//                           // white - power +5v
//                           // brown - power GND
#include <avr/interrupt.h> //DONT INCLUDE UNLESS YOU NEED

#include <util/delay.h>


uint8_t cursor_x = 0;
uint8_t cursor_y = 0;

//#include <avr/pgmspace.h>

/*
  The machine shall be called "Kernel Splatter." (Inspired by Red Lobster adrmiral's platter)
  Because Kommodore 64 seemed a bit pretentious.
  -----------------------

  This code started out as an experiment for I/O to a UM61512AK SRAM chip.
  Then I added a UART -> write/read SRAM data.
  Then it became a "bootloader" for a romless, all in sram, 6502 computer system.
 
  -----------------------
 
  Notes - 

   I oringially was controlling OE and CS on SRAM with software to access memory. 
   
   After studying the loom 6502 SBC - it appears that you can simply wire OE, and CS low 
   and toggle WE alone to write to ram. I am going to comment out the code that toggles the 
   other two and use them for 6502 reset and clock instead. 

   --
   Dont operate on SRAM with AVR while 6502 is clocked or doing anything!
   --
   noticed serial port behaving different when background serial term running - look into this!
   --

   The SRAM is wired to only be writable when PHI2 (clock inverted OUT) is high (clock LOW = phi2 high) . 
   You have to NOT be clocking the 6502 during ram access from AVR 
   Put (PHI0?) clock in LOW position when loading boot code over serial 
   --
   
   Reminder - any probes/LEDS/wires hooked up to bus will cause gremlins in yer bitz.
   DONT RUN WITH WIRES HOOKED UP TO ADDRESS OR DATA BUS!!!!

*/

/***********************************************/
/***********************************************/

//TOY_CTRL_BUS - CONTROL BUS - bits 1-3

//                          SRAM   | 6502     | AVR 
//------------------------------------------------   
#define WE_PIN 0x01       //29  WE | 34 R/W   | PD0
#define CLK_PIN 0x02      //       | 37 phi0  | PD1
#define SRAM_CE 0x04      //30 CE2 | 39 phi2  | PD2
#define MOS6502_RST 0x08  //       | 40 reset | PD3  

#define LED_RGB_PORT PORTH


#define TOY_CTRL_BUS PORTD
#define TOY_CTRL_DDR DDRD

#define TOY_DATA_BUS PORTK
#define TOY_DATA_DDR DDRK

#define TOY_ADDR_BUSMSB PORTC
#define TOY_ADDR_MSBDDR DDRC
#define TOY_ADDR_BUSLSB PORTL
#define TOY_ADDR_LSBDDR DDRL

/***********************************************/
/* * * * known commands * * * */

//6502 core system
#define CMD_RST_6502 0x72        //r - reset 6502
#define CMD_HALT_6502 0x68       //h - halt 6502 clocking
#define CMD_RESUM_6502 0x63      //c - resume 6502 clocking 

//"boundary scan" vision of features
#define CMD_SET_ADDR_BUS 0x61    //a  2 bytes in
#define CMD_GET_ADDR_BUS 0x62    //b  2 bytes out
#define CMD_SET_DATA_BUS 0x65    //e  1 byte in  
#define CMD_GET_DATA_BUS 0x66    //f  1 byte out


#define CMD_NES_DRAW 0x6e        //n 
#define CMD_CLR_SCRN 0x4e        //N 
#define CMD_BITMAP 0x42          //B  //DRAW SRAM DIRECTLY
#define CMD_BITMAP2 0x43         //C  //copy AVR-SRAM DRAW SRAM
#define CMD_BITMAP3 0x44         //D  //draw AVR BITMAP

//uart related
#define CMD_ECHO 0x71            //q - test serial port echo 
#define CMD_TX_SRAM 0x64         //d - download sram from 6502
#define CMD_RX_SRAM 0x75         //u - upload sram into 6502

//memory related
//#define CMD_SET_MEMLOC 0x6a    //j  3 bytes in        (addr X2,  data)  
//#define CMD_GET_MEMLOC 0x6b    //k  1 byte out
#define CMD_PEEKRANGE 0x50       //P - look at memory range (2X16bit     args)       
#define CMD_PEEK_6502 0x70       //p - look at memory       (2X8bit      args)
#define CMD_POKE_6502 0x6f       //o - set memory           (1X16bit 1X8 args) 
//#define CMD_POKERANGE 0x50     //P - look at memory range (2X16bit     args)   

#define CMD_FREERUN 0x7a         //z - flood RAM with NOP's
#define CMD_ZEROSRAM 0x78        //x - flood RAM with 0's
#define CMD_TESTSRAM 0x74        //t - flood RAM with index%256 

#define CMD_SHOWINTERNALS 0x73     //s - show global numeric buffer  
#define CMD_SET_INTERNALS 0x53     //S - set  global numeric buffer  (4 bytes?) 
#define CMD_SET_SPRITE_XY 0x54    //T - set  global numeric buffer  (2 bytes)    

//#define CMD_TOGGLE_TERMODE 0x54 //T - toggle terminal mode 



/* * * * known commands * * * */
/***********************************************/

//standard buffers for 8 to 16 bit stitching and unwrapping
//often we just need 3 bytes (2 addr+1 data) in that case ignore com_msb2
uint8_t buf1_lsb = 0;
uint8_t buf1_msb = 0;
uint8_t buf2_lsb = 0;
uint8_t buf2_msb = 0;

uint16_t buf1_16 = 0;
uint16_t buf2_16 = 0;

uint16_t sprite_xy = 0; //packed value two bytes 


//STATES FOR MACHINE BIOS 
uint8_t ishalted = 1;       //start out halted

//in NON terminal mode - send ACK after each command?
uint8_t terminal_mode = 1;  //be more chatty over serial for command feedback to humans (instead of python)


/***********************************************/
/*
   send a byte over serial port represented as text - 1's and 0's 
*/

void send_txt_byte( uint8_t data, uint8_t use_newline){
   uint8_t i = 0;

   for (i=0; i<=7; i++) {
       //if ( !!(data & (1 << ii)) ){  // LSB
       if ( !!(data & (1 << (7 - i))) ){  // MSB
           UART_Transmit( BIT_OFF );
       }else{
           UART_Transmit( BIT_ON );
       }
    }

    if(use_newline!=0){
        UART_Transmit( 0xa ); //CHAR_TERM = new line  
        UART_Transmit( 0xd ); //0xd = carriage return
    }
}

/***********************************************/
//for printing internal 16 bit numbers - all serial related I/O is bytesX2
void send_txt_2bytes( uint16_t data, uint8_t use_newline,  uint8_t use_space){
   uint8_t i = 0;

   for (i=0; i<=15; i++) {
       //if (i==8){  UART_Transmit(0x20); }//middle space 

       if ( !!(data & (1 << (15 - i))) ){  // MSB
           UART_Transmit( BIT_OFF );
       }else{
           UART_Transmit( BIT_ON );
       }
    }
    
    if(use_space!=0){
        UART_Transmit(0x20);    //SPACE 
    }

    if(use_newline!=0){
        UART_Transmit( 0xa ); //CHAR_TERM = new line  
        UART_Transmit( 0xd ); //0xd = carriage return
    }
}


/***********************************************/
void UART_Init( unsigned int ubrr)
{
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    
    //DOH !! RUN AT FULL SPEED 115K!!!
    UCSR0A |= (1<<U2X0); //DAMN DAMN DAMN - HOW DID I NOT FIND THIS SOONER????

}

/***********************************************/
//static uint8_t UART_receive(void)
uint8_t UART_receive(void)
{
    while (!(UCSR0A & (1 << RXC0))) {}
    return UDR0;
}
/***********************************************/
void UART_Transmit( unsigned char data )
{
  while ( !( UCSR0A & (1<<UDRE0)) );
  UDR0 = data;
}

//*******************************************//
void UARTWriteStr(char *data) 
{ 
   while(*data){ 
      UART_Transmit(*data++); 
   }
}


//*******************************************//
unsigned char y[20]; // in main
unsigned char len;

unsigned char uartrecieve(unsigned char *x, unsigned char size)
{
    unsigned char i = 0;

    if (size == 0) return 0;              // return 0 if no space

    while (i < size - 1) {                // check space is available (including additional null char at end)
        unsigned char c;
        while ( !(UCSR0A & (1<<RXC0)) );  // wait for another char - WARNING this will wait forever if nothing is received
        c = UDR0;
        if (c == '\0') break;             // break on NULL character
        x[i] = c;                         // write into the supplied buffer
        i++;
    }
    x[i] = 0;                             // ensure string is null terminated

    return i + 1;                         // return number of characters written
} 

//len = uartrecieve(y, 20);





/***********************************************/
//debug experimental - cant seem to make this work!
void UART_tx_string( char data[]  )
{
    int i = 0; 
    while(data[i] != '\0')
    {
        UDR0 = data[i++];
        while (!(UCSR0A & (1 <<UDRE0)));
    }

    while (!(UCSR0A & (1 <<UDRE0)));
    UDR0 = '\0';
}





/***********************************************/
//interrupt based clock pulses 
/*
void init_timer_interrupts(void){
    TCCR1B |= (1 << WGM12 ) ; // Configure timer 1 for CTC mode
    TIMSK1 |= (1 << OCIE1A ) ; // Enable CTC interrupt
    sei();  
    OCR1A = 1;  
    TCCR1B |= ( (1 << CS10 )  ) ; // Start timer at Fcpu /64
}
*/

//test for interrupt to halt on UART recieve
 
//Writing this bit to one enables interrupt on the RXCn Flag. A UART Receive Complete interrupt will be generated
//only if the RXCIEn bit is written to one, the Global Interrupt Flag in SREG is written to one and the RXCn bit in
//UCSRnA is set.

void init_uart_interrupt(){
   UCSR0B |= (1 << RXCIE0); // Enable the UART Recieve Complete interrupt (UART_RXC) 
   //sei(); // Enable the Global Interrupt Enable flag so that interrupts can be processed 
}



/***********************************************/
void echo_uart(void){
    uint8_t buf = UART_receive();
    send_txt_byte(buf, 1);
    scribe_byte_astext(buf);
}

/****************************************************************************************************************/
/****************************************************************************************************************/
/****************************************************************************************************************/


//I never did figure out char mapping but it uses this:
//https://en.wikipedia.org/wiki/Code_page_437 
void scribe_str(char *data) 
{ 
   //uint8_t cursor_x = 0;
   //uint8_t cursor_y = 0;
  
   cursor_y+=10;
   if(cursor_y>=160){
       cursor_y=0;
       ST7735_FillScreen(1);
   }

   uint8_t ct = 0;
   while(*data){ 
      //ST7735_DrawCharS(8*ct, 10, *data++, 0x0ff0, 0x0000, 1);
      ST7735_DrawCharS(ct*7, cursor_y, *data++, 0x0ff0, 0x0000, 1);
      
      ct++; 
   }

}

//*******************************************//
/*
   analog of send text byte - write a byte to screen as 1's and 0's
*/
void scribe_byte_astext( uint8_t data) 
{ 
   uint8_t ct = 0;
   uint8_t i = 0;
   uint8_t ypos = y;

   cursor_y+=10;
   if(cursor_y>=160){
       cursor_y=0;
       ST7735_FillScreen(1);
   }

   for (i=0; i<=7; i++) {
       if ( !!(data & (1 << (7 - i))) ){  // MSB
           ST7735_DrawCharS(i+(ct*7), cursor_y, BIT_OFF, 0x33f0, 0x0000, 1);
       }else{
           ST7735_DrawCharS(i+(ct*7), cursor_y, BIT_ON , 0x33f0, 0x0000, 1);
       }
       ct++;//advance "cursor"
    }
 
}

//*******************************************//
void scribe_byte2_astext( uint16_t data) 
{ 
   uint8_t ct = 0;
   uint8_t i = 0;
   uint8_t ypos = y;

   cursor_y+=10;
   if(cursor_y>=160){
       cursor_y=0;
       ST7735_FillScreen(1);
   }

   for (i=0; i<=15; i++) {
       if ( !!(data & (1 << (15 - i))) ){  // MSB
           ST7735_DrawCharS(i+(ct*7), cursor_y, BIT_OFF, 0xf0ff, 0x0000, 1);
       }else{
           ST7735_DrawCharS(i+(ct*7), cursor_y, BIT_ON , 0xf0ff, 0x0000, 1);
       }
       ct++;//advance "cursor"
    }
 
}


//*******************************************//




/***********************************************/

//lets you set an "index" to a bit - silly little reminder of how to do this sinmple task in code
uint8_t idx_to_byte(uint8_t idx){
    return (1 << idx);
} 

/***********************************************/
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

/***********************************************/
uint16_t cvt_8x2_to_16(uint8_t msbin, uint8_t lsbin){
   uint8_t dat_8;
   uint16_t dat_16;
   dat_8 = msbin;
   dat_16 = (lsbin<<8);
   return (dat_16 | dat_8);
}


/***********************************************/

/***********************************************/
//walk each bit and flash for testing wiring, etc 
void test_addr_bits(void){
    int a=0;
    for(a=0;a<8;a++){
       TOY_ADDR_BUSLSB = idx_to_byte(a);
       _delay_ms(100);
    }
    TOY_ADDR_BUSLSB = 0x00;
    for(a=0;a<8;a++){

       TOY_ADDR_BUSMSB = idx_to_byte(a);
       _delay_ms(100);
    }       
    TOY_ADDR_BUSMSB=0x00;
}
/***********************************************/
/*
   this sets the address bus on the 6502 to an address 
   give it an interger between 0-65535 and it sets the 16 address lines to that value 

*/

void set_address(uint16_t address){
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
    
    TOY_ADDR_BUSLSB = low_byte;
    TOY_ADDR_BUSMSB = high_byte;
}


/***********************************************/
/*
   read a location in SRAM 
*/

uint8_t read_ram(uint16_t address){
    
    uint8_t out = 0x00;

    TOY_DATA_DDR = 0x00; 
    TOY_DATA_BUS = 0x00; //clear data port
    set_address(address);
    TOY_CTRL_BUS |= WE_PIN; //WE high when reading 
    out = PINK;
    TOY_DATA_BUS = 0x00; //clear data port
    return out;
}

/***********************************************/
/*
  write to a location in SRAM 
*/

void write_ram(uint16_t address, uint8_t byte){
    
    TOY_DATA_DDR = 0xff;
    TOY_CTRL_BUS |= WE_PIN; 
    set_address(address);
    TOY_DATA_BUS = byte;
    TOY_CTRL_BUS &= ~WE_PIN;     
    TOY_CTRL_BUS |= WE_PIN;  
}



/*
//THOUGHT THESE WORK - NOW NOT SURE OF ANYFUCKING THING 

uint8_t read_ram(uint16_t address){
    uint8_t out = 0x00;
    TOY_DATA_DDR = 0x00; 
    set_address(address);
    TOY_CTRL_BUS |= WE_PIN; //WE high when reading 
    out = PINK;
    return out;
}
//THOUGHT THESE WORK - NOW NOT SURE OF ANYFUCKING THING 
  
void write_ram(uint16_t address, uint8_t byte){
    
    TOY_DATA_DDR = 0xff;
    TOY_CTRL_BUS |= WE_PIN; 
    set_address(address);
    PORTK = byte;
    TOY_CTRL_BUS &= ~WE_PIN;     
    TOY_CTRL_BUS |= WE_PIN;  
}
*/


/***********************************************/
/*
   Boundary scan like feature to get/set address and data buses 
*/

uint8_t get_dbus(void){
    TOY_DATA_DDR = 0x00; 
    uint8_t tmp = PINK;
    TOY_DATA_DDR = 0xff; 
    return tmp;
}

/***********************************************/
/*
   Boundary scan like feature to get/set address and data buses 
*/

void set_dbus(uint8_t byte){
    TOY_DATA_DDR = 0xff; 
    TOY_DATA_BUS = byte;
    //TOY_DATA_DDR = 0x00; 
}

/***********************************************/
/*
   Boundary scan like feature to get/set address and data buses 
*/

uint16_t get_abus(void){
    return cvt_8x2_to_16(TOY_ADDR_BUSMSB, TOY_ADDR_BUSLSB);
}

/***********************************************/
/*
   Boundary scan like feature to get/set address and data buses 
*/

void set_abus(uint16_t address){
    set_address(address);
}


/***********************************************/
/*
   DEBUG - NOT DONE YET 
   walk each location in SRAM and send it over UART 
   defaults ( start = 0, numbytes 65536 )

         //high byte, shift all bits 8 places right
         //high_byte = (uint8_t)(x >> 8);
         //low byte, clear the high byte
         //low_byte = (uint8_t)(x & 0x00FF);

*/

void tx_bytes(uint16_t startaddr, uint16_t numbytes)
{
     uint16_t b = 0;

     for(b=startaddr;b<numbytes;b++)
     {
         uint8_t tmp = read_ram(b);
         while ( !( UCSR0A & (1<<UDRE0)) );
         UDR0 = tmp;

         //flash LEDS to indicate we are working on sending data 
         if(b%512==0){
             LED_RGB_PORT^=0x02;//red 
         } 
         if(b%2048==0){
             LED_RGB_PORT^=0x01; //green  
         } 
         
     }//B-Y
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
          UART_Transmit(0x61); // a
          UART_Transmit(0x20); //space
          send_txt_byte( (uint8_t)(a >> 8) , 0);//LSB of address
          send_txt_byte( (uint8_t)(a & 0x00FF), 0);//MSB of address
          UART_Transmit(0x20); //space      
          UART_Transmit(0x64); // d
          UART_Transmit(0x20); //space
          send_txt_byte( read_ram(a), 1);//data
       }
   }

}

/***********************************************/
/*
  serial port bootloader. Slow, clunky, and I love it!
  fill local SRAM with data sent over uart from host machine 
*/

void rx_ram(uint16_t numbytes){
   uint16_t a = 0;
   LED_RGB_PORT = 0x00;  //clear LEDS
   LED_RGB_PORT |= 0x02; //turn on green - (setting the first spinlocked byte as RED)
   
   //clamp at 65535
   for(a=0;a<numbytes;a++)
   { 
      
      //flash red to indicate we are working on loading data 
      if(a%2048==0){
          LED_RGB_PORT^=0x01; LED_RGB_PORT^=0x02; //toggle both red and green   
      } 
 
       write_ram(a, UART_receive() ); //pack 16 bit value into 1 byte to send 
   }
  
   LED_RGB_PORT=0x02; 
}

/***********************************************/
void flood_sram(uint8_t byte){
   uint16_t a = 0;   
   for(a=0;a<65535;a++){ 
      if(a%16384==0){
          LED_RGB_PORT^=0x01;LED_RGB_PORT^=0x02;   
      } 
      write_ram(a, byte ); 
   } 
}

/***********************************************/
void clear_sram(void){
    //send message to indicate we have finished initialization
    if (terminal_mode){
        UARTWriteStr("SRAM Initializing.");
    }

   //clear entire memory space 
   flood_sram(0x00);
}
/***********************************************/
void init_toy_computer(void){
   //activate registers to access SRAM and LEDs
   // be sure to set back to HI-Z when starting 6502 up!
   TOY_ADDR_MSBDDR = 0xff;     
   TOY_CTRL_DDR = 0x07;  
   TOY_ADDR_LSBDDR = 0xff; 
   //TOY_DATA_DDR IS SET DYNAMICALLY 

   DDRH = 0x03; //red and green LEDs 0x02 green, 0x01 red

   //setup serial port communication
   UART_Init(MYUBRR);

   //SET INITIAL PIN STATES FOR CONTROL BUS 
   TOY_CTRL_BUS |= CLK_PIN; //ram only can be written to when this is high (phi2 pin is inverted) 
   TOY_CTRL_BUS |= SRAM_CE; //high on sram pin 30 is wirte to RAM 

   //DEBUG - NORMALLY YOU WANT THIS, BUT I ADDED BATTERY BACKUP SO I WANT TO TEST THAT IT KEEPS
   //clear_sram();
   //DEBUG - NORMALLY YOU WANT THIS, BUT I ADDED BATTERY BACKUP SO I WANT TO TEST THAT IT KEEPS

   if (terminal_mode){
        UARTWriteStr("System Ready."); 
        scribe_str("System Initialized.");

   }
   LED_RGB_PORT =0x01; //red led  

}

/***********************************************/

/* COM buffer is for command args - Serial buffer is for stiching and tearing,
   helper to grab arguments for commands over serial port 
*/

  
uint16_t rx_two_bytes(void){
    uint8_t c1 =  UART_receive();//lsb addr
    uint8_t c2 =  UART_receive();//msb addr
    //send_txt_2bytes(buf1_16, 1, 0);
    scribe_str("2 bytes received.");
    return cvt_8x2_to_16(c1, c2); //fills buffer buf1_16

}
/***********************************************/
 
 //helper to grab arguments for commands over serial port 
 void rx_three_bytes(void){
    uint8_t c1 =  UART_receive();//lsb addr
    uint8_t c2 =  UART_receive();//msb addr 
    buf2_lsb =  UART_receive();//data byte 

    buf1_16 = cvt_8x2_to_16(c1, c2); //fills buffer buf1_16
    scribe_str("3 bytes received.");
 }
 
/***********************************************/
//helper to grab arguments for commands over serial port 
void rx_four_bytes(void){
    // uint8_t c1 =  UART_receive();//lsb addr
    // uint8_t c2 =  UART_receive();//msb addr 
    // uint8_t d1 =  UART_receive();//lsb addr
    // uint8_t d2 =  UART_receive();//msb addr 
    // cvt_8x2_to_16(d1, d2); //fills buffer buf1_16
    // buf2_16 = buf1_16;     //copy buffer 
    // cvt_8x2_to_16(c1, c2); //fills buffer buf1_16
    // send_txt_2bytes(buf1_16, 1, 0);
    // send_txt_2bytes(buf2_16, 1, 0);
    
    buf1_lsb =  UART_receive();//lsb addr
    buf1_msb =  UART_receive();//msb addr 
    buf2_lsb =  UART_receive();//lsb addr
    buf2_msb =  UART_receive();//msb addr 
    scribe_str("4 bytes received.");
}


/***********************************************/
void halt_6502(void){

   //these should only be electrically active when 6502  is running 
   TOY_ADDR_MSBDDR = 0xff;     
   TOY_CTRL_DDR = 0x1f; ////bottom 5 bits as I/O
   TOY_ADDR_LSBDDR = 0xff;

   //send message to indicate we have halted
   if (terminal_mode){
        UARTWriteStr("System Halted."); 
   } 
   ishalted =1;
}

//resume is start - same thing
void resume_6502(void){
   
   //set all AVR pins to HIZ to avoid bus contention 
   TOY_ADDR_MSBDDR = 0x00;     
   TOY_ADDR_LSBDDR = 0x00;
   TOY_DATA_DDR = 0x00;
   TOY_CTRL_DDR = 0xf6; // 11110110 , (all but reset pin runs but no memory write)

   LED_RGB_PORT=0x00; //clear all leds

   //send message to indicate we have finished initialization
   if (terminal_mode){
        UARTWriteStr("System Resume.");  
   }
   //TOY_CTRL_BUS |= 0x08; //reset high (active low) 

    /* USE A TIMER INTERRUPT INSTEAD SO IT WONT LOCK UP
    while(1){
        LED_RGB_PORT=0x00;
        _delay_ms(400);
        LED_RGB_PORT=0x02;
        _delay_ms(2);        
    }*/
  ishalted = 0;
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
void test_sram(void){
    flood_pattern(); 
    uint16_t y =0;
    for(y=0;y<20;y++){
        send_txt_byte( read_ram(y) ,1);
    }    
}

/***********************************************/
void reset_6502(void){
    //THIS ONLY PAUSES , NOT RESETS?? I DONT KNOW?
    //maybe AVR cant pull the line low enough since it is pulled up via resistor?

    //set TOY_CTRL_BUS bit 4 to OUTPUT 
    TOY_CTRL_DDR |= MOS6502_RST;
    TOY_CTRL_BUS &= ~MOS6502_RST;//set low

    //IF YOU DONT PULSE CLOCK IT JUST PAUSES
    TOY_CTRL_BUS ^= CLK_PIN;
    TOY_CTRL_BUS ^= CLK_PIN;
    TOY_CTRL_BUS ^= CLK_PIN;
    TOY_CTRL_BUS ^= CLK_PIN;
    TOY_CTRL_BUS ^= CLK_PIN;
    TOY_CTRL_BUS ^= CLK_PIN;

    TOY_CTRL_DDR &=~MOS6502_RST;//set back to run 
}


/***********************************************/
/*
   This sends a clock singal to 6502. It will be much slower than a real crystal 

   neither this nor interrupt clock pulses are ideal
   this is much faster but you cant stop it, interrupts dont pulse correctly 
   unless perhaps you much with timing  a bunch 
*/

uint16_t led_cnt = 0; //use for fancier LED blinking 
void spinclock(void){
    sei(); //turn on interrupts so we can listen for halt signal

    while(ishalted!=1){ 

       if (ishalted==1){
          return;
       }
       TOY_CTRL_BUS ^= CLK_PIN;
       //_delay_us(2); //1 Mhz clock
       //if(led_cnt<100){LED_RGB_PORT = 0x02;}//blink led @ 1Mhz
       _delay_ms(10);//much slower than 1 Mhz!  
       if(led_cnt>100){LED_RGB_PORT = 0x02;led_cnt=0; }//blink led @ slow speed  
       else {LED_RGB_PORT=0x00;led_cnt++;}              //blink led @ slow speed            

    }
}

/***********************************************/

void listen_serial()
{
    //heartbeat switches for custom blinkies
    uint8_t isknown = 0;
    uint8_t leavered = 0;
    uint8_t leavegreen = 0;

    ////

    while(1)
    {
        isknown = 0;//set color of heartbeat LED
        while (!(UCSR0A & (1 << RXC0))) {}//Busy wait.

       //  if (ishalted){
       //      switch(UDR0)
       //      { 
       //          case CMD_RESUM_6502: //c  
       //              resume_6502();
       //              isknown=1;
       //              leavered=0;   
       //          break;
       //      }
       //  }else{
       uint8_t st_px = 0;
       uint8_t ed_px = 0;
       
       if(1){ //USE ABOVE CODE LATER FOR HALT CONDITIONS
                switch(UDR0)
                {

                    case CMD_RST_6502: //r  - reset 6502
                        reset_6502(); //not working right 
                        isknown=1;            
                    break;

                    /**********/
                    case CMD_NES_DRAW: //n
                        testNES2(); 
                    break;

                    /**********/
                    case CMD_CLR_SCRN: //N
                           ST7735_FillScreen(0); //clear screen black ; 
                           cursor_y = 0;
                           cursor_x = 0;              
                    break;
                    
                    /**********/
                    case CMD_BITMAP: //B
                        //TEST - SEND START AND END LOCATIONS OVER IN COMPRESSED 16 BIT VALUE  
                        st_px = (uint8_t)(sprite_xy >> 8);     //LSB of address
                        ed_px = (uint8_t)(sprite_xy & 0x00FF); //MSB of address

                        ST7735_DrawBitmapSRAM(st_px, ed_px, 0, 10, 10);
                    break;

                    /**********/
                    case CMD_BITMAP2: //C
                        blit_test();
                    break;           


                    /**********/
                    case CMD_BITMAP3: //D
                        bitmap_test();
                    break; 
                             

                    /**********/

                    case CMD_SHOWINTERNALS: //s  - show globals
                        isknown=1;            
                        
                        //cvt_8x2_to_16(buf1_lsb, buf1_msb);
                        send_txt_2bytes( buf2_16, 0, 1);
                        //cvt_8x2_to_16(buf2_lsb, buf2_msb );
                        send_txt_2bytes( buf1_16, 1, 1);
                        // uint8_t ishalted = 1;  
                    break;
                    /**********/

                    //terminal_mode = 1; //TOGGLE TERMINAL MODE - RUN IN SINGLE TERMINAL

                    //case CMD_TOGGLE_TERMODE:
                    //break;

                    /**********/
                    case CMD_SET_SPRITE_XY: //T - set two bytes internally
                        isknown=1;            
                        sprite_xy = rx_two_bytes(); 
             
                        //TO GET DATA: 
                        // (uint8_t)(buf1_16 >> 8);     //LSB of address
                        // (uint8_t)(buf1_16 & 0x00FF); //MSB of address
                    break;

                    case CMD_SET_INTERNALS: //S  - set globals
                        isknown=1;            
                        rx_four_bytes(); 
             
                        buf2_16 = cvt_8x2_to_16(buf1_lsb, buf1_msb);
                        buf1_16 =cvt_8x2_to_16(buf2_lsb, buf2_msb );
                    break;

                    /**********/
                    case CMD_SET_ADDR_BUS: //a
                        buf1_16 = rx_two_bytes(); 
                        send_txt_2bytes(buf1_16, 1, 0);
                        set_abus(buf1_16);
                    break;
                    
                    case CMD_GET_ADDR_BUS: //b  
                        //send_txt_2bytes(buf1_16, 0 ,0);
                    break;
                    
                    case CMD_SET_DATA_BUS: //e  
                        //set_dbus(UART_receive());
                    break;
                    
                    case CMD_GET_DATA_BUS: //f 
                        send_txt_byte(get_dbus(), 1);
                    break;

                    /**********/
                    case CMD_ECHO: //q
                        _delay_us(300);
                   
                        echo_uart();
                        UART_Transmit( 0x0d ); // CR          
                        UART_Transmit( 0x0a );
                        isknown=1;
                    break;

                    /**********/

                    //upload SRAM "disk image" (compy-> 6502) 
                    case CMD_RX_SRAM: //u  
                        scribe_str("loading SRAM dump.");                    
                        rx_ram(65535);
                        scribe_str("done.");                         
                        //show_ram_region(65300, 65400);
                        //_delay_ms(100);
                        //resume_6502();
                        
                        isknown=1;               
                    break;

                    //download SRAM "disk image" (6502->compy)
                    case CMD_TX_SRAM: //d  
                        _delay_ms(100); 
                        scribe_str("saving SRAM dump.");
                        tx_bytes(0, 65535);
                        scribe_str("done.");                        
                        // UART_Transmit( 0x42 );           
                        // UART_Transmit( 0x0d ); // CR          
                        // UART_Transmit( 0x0a );            
                        isknown=1;

                    break;

                    case CMD_PEEK_6502: //p 
                        //peek a 16 bit location from 2 bytes 
                        buf1_16 = rx_two_bytes(); 
                     
                        UART_Transmit(0x61);                          // a
                        UART_Transmit(0x20);                          //space
                        send_txt_byte( (uint8_t)(buf1_16 >> 8) , 0);     //LSB of address
                        send_txt_byte( (uint8_t)(buf1_16 & 0x00FF), 0);  //MSB of address
                        UART_Transmit(0x20);                          //space      
                        UART_Transmit(0x64);                          // d
                        UART_Transmit(0x20);                          //space 
                        send_txt_byte( read_ram(buf1_16), 1);            // data byte
                        isknown=1; 

                        //scribe_str(".........")
                        scribe_byte2_astext(buf1_16);            //SHOW ADDR 
                        scribe_byte_astext( read_ram(buf1_16) ); //SHOW DATA

                    break;

                    case CMD_PEEKRANGE: //P  - NOT DONE
                        //peek a range of addresses - 2 bytes start , 2 bytes stop 
                        isknown=1; 
                        rx_four_bytes(); 
             
                        buf2_16 = cvt_8x2_to_16(buf1_lsb, buf1_msb);
                        buf1_16 = cvt_8x2_to_16(buf2_lsb, buf2_msb );
                        show_ram_region(buf2_16, buf1_16);
                    break;

                    //PEEK AND POKE ARE UNTESTED!
                    case CMD_POKE_6502:     //o - set memory (3X8bit args)  
                        rx_three_bytes();
                        write_ram(buf1_16, buf2_lsb);// buf1_16= addr  buf2_lsb = data
                        
                        //send_txt_2bytes(buf1_16, 0, 0) ;      //sends 19 bytes with both set to 1
                        //send_txt_byte( read_ram(buf1_16), 0 ); 

                        isknown=1;            
                    break;

                    /**************************************/
                    //pretty sure this isnt the way to properly do this  
                    case CMD_RESUM_6502: //c  
                        resume_6502();
                        isknown=1;
                        leavered=0; 
                        
                        scribe_str("clocking 6502 CPU");
                        //you can use external clock and not lock the bugger up 
                        spinclock();//run clock forever  

                    break;

                    //pretty sure this isnt the way to properly do this           
                    case CMD_HALT_6502: //h  
                        scribe_str("halt");
                        halt_6502();
                        isknown=1; 
                        leavered=1; 
                    break;

                    /**********/
                    /**************************************/
                    /**********/

                    case CMD_FREERUN: //z  - flood ram with NOPs
                        scribe_str("flood SRAM NOP's");                    
                        isknown=1;
                        flood_sram(0xea);//NOP byte 0xea  
                        scribe_str("done.");           
                    break;

                    case CMD_ZEROSRAM: //x  - flood ram with 0's
                        scribe_str("clear SRAM");
                        isknown=1;  
                        clear_sram(); 
                        scribe_str("done.");           
                    break;
                    /**********/

                    // UNTESTED!
                    case CMD_TESTSRAM: //t  - flood ram index%256
                        isknown=1;  
                        flood_pattern();           
                    break;

                    /**********/

                }
        }//else (not halted)

        ////             ////

        //This is just for some cool info displayed in a heartbeat LED blinky            
        if(leavered){
            LED_RGB_PORT=0x01;
        }else if(leavegreen){
            LED_RGB_PORT=0x02;
        }else{
          //heartbeat
          LED_RGB_PORT = 0x00;
          if(isknown){LED_RGB_PORT|=0x02;}else{LED_RGB_PORT|=0x01;}  
          _delay_ms(20);
          LED_RGB_PORT =0x00; 
        }

    }//while    

}



/***********************************************/

//interrupts work well for clock because you can stop,
// but it fuqs up all the timing. They load AVRs stack and add spikes in the timing

/*
ISR ( TIMER1_COMPA_vect )
{
    if(ishalted){
        //TOY_CTRL_BUS =0x00;
    }else{
        TOY_CTRL_BUS ^= CLK_PIN; 
    }
}*/

/***********************************************/

/*
ISR( UART0_RX_vect)
{
    char ReceivedByte;
    ReceivedByte = UDR0; 

    //?? When interrupt-driven data reception is used, the receive complete routine must read the 
    //?? received data from UDR in order to clear the RXC Flag, otherwise a new interrupt will occur 
    //?? once the interrupt routine terminates.
    if (ReceivedByte==0x68){
        // Code to be executed when the UART receives a byte here
        PORTH ^= 0x01;
        ishalted=1;  
        cli();//immediately turn off interrupts when halted      
    }
}
*/ 

/************************************************/

void SPI_Init(void)
{
   SPI_DDR = 0xff;   //hardware SPI lines on PORTB
   CTRL_DDR = 0xff;  //software controlled SPI/CS lines on PORTF
   //SPCR |= (1<<MSTR)|(1<<SPE)|(1<<CPHA)|(1<<CPOL);  // SPI Master, SPI Enable, Trailing edge
   SPCR |= (1<<MSTR)|(1<<SPE);  // SPI Master, SPI Enable
   SPSR |= SPI2X;
}






//*******************************************//
//Recieve buffer test
/*
  Each time you call this function, it will overwrite the previous contents of rx_buffer 
  so make sure you are finished using it first. Read up on arrays, pointers and strings 
  if you're not certain what's happening here.

  Better yet would be to pass in a pointer to uartreceive so that the calling function 
  can supply the area of memory

  2. It is unlikely that your serial terminal software will be sending NULL terminated 
  strings by default (i.e. with '\0' at the end), normally it would send a new-line 
  ('\n') character. I believe realterm can do this, but it's worth checking.

  3. Reading from UDR will clear the RXC flag, allowing the AVR to write another 
  character into UDR, so reading from UDR twice in a row is probably a bad idea
*/

  //*******************************************//

  /*
  if (REC != 0) {
    if (REC == '\n') {
        // do whatever you want with the array contents
        i =0;
        data[0] = 0;
    } else {
        data[i++] = REC;
    }
    REC = 0;
} 
*/




//*******************************************//

/*
//#define HEX_TO_DEC_ERROR 42 
unsigned char hex_to_dec(unsigned char in) 
{ 
   if(((in >= '0') && (in <= '9'))) return in-'0'; 
   in |= 0x20;
   if(((in >= 'a') && (in <= 'f'))) return in-'a' + 10; 
   return 42; 
}
*/



//*******************************************//
/*
   Bit bang a shift register interface for an old school NES controller 
   used 3 data lines 
   serial data, serial clock , and data latch
*/

void NES_CTRLR_Init( void )
{
    nes_gpio_ddr &=~ nes_data_pin; //make inputs
}


//*******************************************//
uint8_t nes_controller_read(void)
{
  uint8_t controller_data = 0;
  uint8_t i = 0;
  uint8_t latchtime = 30;//u_sec delay between clock pulses 

  nes_gpio_port &= ~nes_latch_pin; 
  controller_data = !!(nes_input_port & nes_data_pin); //NES yellow wire

  for (i=0; i<7; i++) {
     nes_gpio_port |= nes_clock_pin;
     _delay_us(latchtime); 
     controller_data = controller_data << 1;
     controller_data = controller_data + !!(nes_input_port & nes_data_pin); 
     nes_gpio_port &= ~nes_clock_pin;
     _delay_us(latchtime); 
  }
  
  nes_gpio_port |= nes_latch_pin;  

  return controller_data;
}

//*******************************************//
void testNES(void)
{
     uint8_t nes_byte = nes_controller_read();
     scribe_byte_astext( nes_byte );
     _delay_ms(100);
}


//*******************************************//
//really simple character drawing progam
void testNES2(void)
{
     uint8_t x = 10;
     uint8_t y = 10;
     uint8_t w = 128;
     uint8_t h = 160;
     uint8_t lastbyte = 0;
     uint8_t symbol = 0x01;
     uint8_t is_neg = 0;

     uint16_t color = 0x0ff0;
   
     ST7735_FillScreen(0); //clear screen black 

     //TEXT FITS IN A 16_TALL X 18_WIDE GRID 
     int a = 0;
     while(1)
     {
         uint8_t nes_byte = nes_controller_read();
         if (lastbyte!=nes_byte)
         { 
             
             //overwrite cursor image that could be inverted with a correct image
             ST7735_DrawCharS(x+(x*7), (y*10), symbol , color, 0x0000, 1); //7 and 10 are spacing for char itself

             if(nes_byte == 0xFE ) { if(x<15){x++;} } //right
             if(nes_byte == 0xFD ) { if(x>0 ){x--;} } //left
             if(nes_byte == 0xFB ) { if(y<15){y++;} } //down
             if(nes_byte == 0xF7 ) { if(y>0 ){y--;} } //up
             if(nes_byte == 0xDF ) { color+=100;    } //select
             if(nes_byte == 0xEF ) { color-=100;    } //start
             if(nes_byte == 0x7F ) { if(symbol>0)   { symbol--;} } //button a
             if(nes_byte == 0xBF ) { if(symbol<128) { symbol++;} } //button b

             //ST7735_FillScreen(1);
             ST7735_DrawCharS(x+(x*7), (y*10), symbol , color, 0x0000, 1); //7 and 10 are spacing for char itself
             lastbyte = nes_byte;
         }else{
             
             //setAddrWindow(x, y-h+1, x+w-1, y);
             if(is_neg){
                 ST7735_DrawCharS(x+(x*7), (y*10), symbol , 0x0000, color, 1); //7 and 10 are spacing for char itself
                 //ST7735_InvertDisplay(1);
                 is_neg=0;
             }else{
                 ST7735_DrawCharS(x+(x*7), (y*10), symbol , color, 0x0000, 1); //7 and 10 are spacing for char itself              
                 //ST7735_InvertDisplay(0);
                 is_neg=1;
             }
         }
     }       
}

/***********************************************/

void bitmap_test(void){
   //DRAW AN IMAGE FROM AVR'S RAM 
   //IMAGE WILL BE UPSIDE DOWN (TOP LEFT ORIGIN)  
   
   //const uint16_t imagedata[] = {   
   //                           0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff 
   //                          ,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff 
   //                          ,0x00ff,0xffff,0x00ff,0x00ff,0xffff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff 
   //                          ,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff 
   //                          ,0x00ff,0x00ff,0xffff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff 
   //                          ,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff 
   //                          ,0xffff,0x00ff,0x00ff,0x00ff,0x00ff,0xffff,0x00ff,0x00ff,0x00ff,0x00ff 
   //                          ,0x00ff,0xffff,0x00ff,0x00ff,0xffff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff 
   //                          ,0x00ff,0x00ff,0xffff,0xffff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff 
   //                          ,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff,0x00ff 
   //                       };
  const uint16_t imagedata[] = {   
      0x42CD, 0x42CD, 0x3A8C, 0x324C, 0x324B, 0x3A8C, 0x42ED, 0x4B0E, 0x42ED, 0x42ED,
      0x4B0E, 0x4B2E, 0x42CD, 0x42AC, 0x42CD, 0x3AAC, 0x3AAC, 0x3AAC, 0x3AAC, 0x3AAC,
      0x4AED, 0x4B0D, 0x4AED, 0x42CD, 0x42CD, 0x3ACD, 0x42CD, 0x42CD, 0x4A4B, 0x5907,
      0x4ACD, 0x4AED, 0x4B0E, 0x4B0D, 0x42ED, 0x42CD, 0x42CD, 0x42CD, 0x42CD, 0x4AED,
      0x3ACD, 0x3A8C, 0x3A8C, 0x3AAC, 0x42CD, 0x42CD, 0x42CD, 0x4AED, 0x42ED, 0x42ED,
      0x4B0D, 0x4ACC, 0x428B, 0x3A6B, 0x3A4A, 0x3A4A, 0x3A4A, 0x3A4A, 0x42AC, 0x326B,
      0x3A6C, 0x326C, 0x326B, 0x324B, 0x2A09, 0x29C9, 0x21A8, 0x29C8, 0x2167, 0x1946,
      0x1945, 0x10C4, 0x2146, 0x2145, 0x2145, 0x2145, 0x2166, 0x2146, 0x42ED, 0x42ED,
      0x42ED, 0x4B0E, 0x4B0D, 0x42ED, 0x4B0E, 0x4B0E, 0x532E, 0x534F, 0x4B0E, 0x4B0D,
      0x3AAD, 0x3AAD, 0x42CD, 0x42ED, 0x3ACD, 0x3A8C, 0x3A8C, 0x3AAD, 0x430D, 0x4B0D
   };
   
   const uint16_t *img = &imagedata;
   
   ST7735_DrawBitmap(20, 20, img, 10, 10);
}


/***********************************************/
void blit_test(void){
   //COPY SRAM DATA FROM FROM AVR'S RAM , THEN DRAW FROM SRAM 
   //THIS IS THE FIRST HALF OF TRANSMIT BMP FUNCTION OVER UART
   //IMAGE WILL BE UPSIDE DOWN (TOP LEFT ORIGIN)  
   const uint16_t imagedata[] = {   
                              0x00aa,0x00ff,0x00aa,0x00ff,0x00ea,0x0ff0,0x00ff,0x00ff,0x00ff,0x00ea 
                             ,0x00ff,0x00ff,0x00ff,0x00ff,0x00ea,0x0ff0,0x00ff,0x00ff,0x00ff,0x00ea 
                             ,0x00ff,0xffff,0x00ff,0x00ff,0x00ea,0x0ff0,0x00ff,0x00ff,0x00ff,0x00ea 
                             ,0x00ff,0x00ff,0x00ff,0x00ff,0x00ea,0x0ff0,0x00ff,0x00ff,0x00ea,0x00ff 
                             ,0x00ff,0x00ff,0xffff,0x00ff,0x00ea,0x0ff0,0x00ff,0x00ff,0x00ea,0x00ff 
                             ,0x00ff,0x00ff,0x00ff,0x00ff,0x00ea,0x0ff0,0x00ff,0x00ff,0x00ea,0x00ff 
                             ,0xffff,0x00ff,0x00ff,0x00ff,0x00ea,0x0ff0,0x00ff,0x00ff,0x00ff,0x00ea 
                             ,0x00ff,0xffff,0x00ff,0x00ff,0x00ea,0x0ff0,0x00ff,0x00ff,0x00ff,0x00ea 
                             ,0x00ff,0x00ff,0xffff,0xffff,0x00ea,0x0ff0,0x00ff,0x00ff,0x00ff,0x00ea 
                             ,0x00aa,0x00ff,0x00ff,0x00aa,0x00ea,0x0ff0,0x00ff,0x00ff,0x00ff,0x00ea 
                          };
   const uint16_t *img = &imagedata;

   /*
   //experiment to procedurally fill ram
   uint16_t imagedata[100] = {};
   uint16_t a = 0;
   for(a=0;a<100;a++){
       imagedata[a]=a;  
   }
   //const uint16_t imagedata[] = {};
   //const uint16_t *img = &imagedata;
   */
   
   blit_SRAM( 0, 0, 0, img, 10, 10);
   ST7735_DrawBitmapSRAM(10, 10, 0, 10, 10);
}

/***********************************************/

void blit_SRAM(uint8_t x, uint8_t y, uint16_t start, const uint16_t *image, uint8_t w, uint8_t h)
{  
  /*
       copy 16 bit data from AVR to SRAM with intention of it drawing a framebuffer 
       x and y allow you to shift it inside its own indexing from a relative start address
  */

   //st_px = (uint8_t)(buf1_16 >> 8);     //LSB of address
   //ed_px = (uint8_t)(buf1_16 & 0x00FF); //MSB of address


  uint16_t idx = 0;

  for(y=0; y<h; y++){
      for(x=0; x<w; x++){
          //debug !
          write_ram( idx, image[idx] & 0x00FF ) ; //this is 8 bit !!
          //write_ram( (y*x)+x, image[(y*x)+x] & 0x00FF ) ; //this is 8 bit !!        
          //debug !
          idx++;
      }
  }
}


/***********************************************/

void ST7735_DrawBitmapSRAM(uint8_t x, uint8_t y, uint16_t start, uint8_t w, uint8_t h){  
  int8_t skipC = 0;                      // non-zero if columns need to be skipped due to clipping
  int8_t originalWidth = w;              // save this value; even if not all columns fit on the screen, the image is still this width in ROM
  uint8_t i = w*(h - 1);

  uint8_t _height = 160;
  uint8_t _width  = 128;


  if((x >= _width) || ((y - h + 1) >= _height) || ((x + w) <= 0) || (y < 0)){
    return;                             // image is totally off the screen, do nothing
  }
  if((w > _width) || (h > _height)){    // image is too wide for the screen, do nothing
    //***This isn't necessarily a fatal error, but it makes the
    //following logic much more complicated, since you can have
    //an image that exceeds multiple boundaries and needs to be
    //clipped on more than one side.
    return;
  }
  if((x + w - 1) >= _width){            // image exceeds right of screen
    skipC = (x + w) - _width;           // skip cut off columns
    w = _width - x;
  }
  if((y - h + 1) < 0){                  // image exceeds top of screen
    i = i - (h - y - 1)*originalWidth;  // skip the last cut off rows
    h = y + 1;
  }
  if(x < 0){                            // image exceeds left of screen
    w = w + x;
    skipC = -1*x;                       // skip cut off columns
    i = i - x;                          // skip the first cut off columns
    x = 0;
  }
  if(y >= _height){                     // image exceeds bottom of screen
    h = h - (y - _height + 1);
    y = _height - 1;
  }

  setAddrWindow(x, y-h+1, x+w-1, y);
  
  uint16_t pixel = 0;
  uint16_t idx    = start;

  for(y=0; y<h; y=y+1){
    for(x=0; x<w; x=x+1){

        pixel = read_ram(idx);
        writedata( pixel ); //msb
        writedata( pixel ); //lsb
        idx++;

        i = i + 1;// go to the next pixel
    }
    i = i + skipC;
    i = i - 2*originalWidth;
  }
}


/***********************************************/
int main (void)
{

   //********************************************
   //setup serial port
   UART_Init(MYUBRR);

   //********************************************
   //init SPI    
   SPI_Init();

   //********************************************
   //init Nintendo Controller
   NES_CTRLR_Init();

   //********************************************
   //init LCD screen 
   ST7735_InitR(INITR_BLACKTAB);  
   ST7735_FillScreen(0); //clear screen black 

   //********************************************
   //init 6502 and SRAM port IO 

   
   init_toy_computer();
   
   //listen_serial(); //<-- main loop

   testNES2();



   //********************************************
   //bitmap_test();





   //********************************************
   //scribe_str( 0, a, "abcdefgABCDEFG0123"); //18 characters
   
   //********************************************
   //scribe_str( 0, a, "abcdefgABCDEFG0123"); //18 characters



}//main


///////////////////////////////////////////////////////////////////////////////////////////////////