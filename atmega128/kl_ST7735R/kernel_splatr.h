
#ifndef _KRNLSPLTR_
#define _KRNLSPLTR_


/*******************************/
//screen properties

#define X_RES 128
#define Y_RES 160

/*******************************/

#define bit_off_color 0x03be //light blue 
#define or_off_color 0xd440  //ORange - or
#define and_off_color 0xd750 //TAN    - and 

#define bit_on_color 0x0ee0  //lime green
#define grid_color 0x2220    //dark grey

#define BIT_ON 0x30 //ascii 1
#define BIT_OFF 0x31 //ascii 0

/*******************************/

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


#define TOY_CTRL_BUS PORTF
#define TOY_CTRL_DDR DDRF

#define TOY_DATA_BUS PORTE
#define TOY_DATA_BUSIN PINE
#define TOY_DATA_DDR DDRE

#define TOY_ADDR_BUSMSB PORTA
#define TOY_ADDR_MSBDDR DDRA
#define TOY_ADDR_BUSLSB PORTC
#define TOY_ADDR_LSBDDR DDRC




//THE DREDEAD  " undefined reference to `USART_receive'" 
//#include "uart.h"
//uint8_t USART_receive(void);




uint16_t rx_two_bytes(void);
void rx_three_bytes(void);
void rx_four_bytes(void);

void halt_6502(void);
void resume_6502(void);
uint8_t get_dbus(void);
void set_dbus(uint8_t byte);
uint16_t get_abus(void);
void set_abus(uint16_t address);
void send_txt_2bytes( uint16_t data, uint8_t use_newline,  uint8_t use_space);
void scribe_str(char *data) ;

void test_up(void);
void test_down(void);
void cursor_loop(void);
void app_one_redraw(void);

void sram_test1(void);
void sram_test2(void);
void sram_erase(void);
void sram_walk(void);

void crsr_up(void);
void crsr_dwn(void);
void crsr_lft(void);
void crsr_right(void);

void right_click(void);
void left_click(void);
void select_click(void);
void start_click(void);


//gfx routines - consider merging with gfx lib?
void draw_outline_box(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);
void draw_byte_box(uint8_t byte, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t offcolor);
void render_8_hex(uint8_t x, uint8_t y, uint8_t size, uint8_t accumulator, uint16_t color);
void render_16_hex(uint8_t x, uint8_t y, uint8_t size, uint16_t accumulator, uint16_t color);

void draw_cursor(uint16_t color);
void clear_screen(uint16_t color);
void bios_l_click(void);
void boot_screen();
void latch_outputs(void);

void listen_serial(void);

#endif