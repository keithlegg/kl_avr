
#include <avr/io.h>

#define F_CPU 16000000UL
#include <util/delay.h>

#include <avr/pgmspace.h>


#include "kernel_splatr.h"

#include "uart.h"
//#include "sram.h"
//#include "nes_ctrlr.h"
//#include "ST7735.h"




uint8_t buf1_lsb;
uint8_t buf1_msb;

uint8_t buf2_lsb = 0;
uint8_t buf2_msb = 0;

uint16_t buf1_16 = 0;
uint16_t buf2_16 = 0;

uint16_t sprite_xy = 0; //packed value two bytes 


//STATES FOR MACHINE BIOS 
uint8_t ishalted = 1;       //start out halted

//in NON terminal mode - send ACK after each command?
uint8_t terminal_mode = 1;  //be more chatty over serial for command feedback to humans (instead of python)

uint8_t cursor_x = 0;
uint8_t cursor_y = 0;


//------------------------------------
//these control widget positions   
volatile uint8_t fontsize      = 1;  

volatile uint8_t byte_width    = 100;
volatile uint8_t step_size_x   = 100/8; //byte_width/8;

volatile uint8_t byte_height   = 10; 
volatile uint8_t step_size_y   = 10;   //same as byte height

volatile uint8_t bb_begin      = 0;    //step_size_x - X coordinate for byte box 
volatile uint8_t hex_begin     = 100;  //X coordinate for hex text 

/*******************************/
//these control coloring  

volatile uint16_t CRSR_COLOR = 0x0ff0; 
//------------------------------------
//these store state 

volatile uint8_t app_idx     = 1; //experiment to switch between "apps"
volatile uint8_t mem_addr    = 0; //bitbanged 16 bit memory address

volatile uint8_t lastx       = 0;
volatile uint8_t lasty       = 0;
volatile uint8_t x           = 0;
volatile uint8_t y           = 0;

volatile uint8_t idx_x       = 7; //count what bit we are on
volatile uint8_t idx_y       = 0; //count what bit we are on

//------------------------------------
//these are the bytes that are rendered 
volatile uint8_t byte1 = 0x0f;
volatile uint8_t byte2 = 0x0f;
volatile uint8_t byte3 = 0x0f;

volatile uint8_t byte4 = 0xff;
volatile uint8_t byte5 = 0xff;
volatile uint8_t byte6 = 0xff;




void crsr_up(void){
    if (y>0){
        y-=step_size_y;
        idx_y--;
    }
    _delay_ms(10); //prevent weirdness    
}
/*******************************/
void crsr_dwn(void){ 
    //if (y<(Y_RES-cursor_size)){
    if (idx_y<10){      
        y+=step_size_y;
        idx_y++;
    }
    _delay_ms(10); //prevent weirdness
}
/*******************************/
void crsr_lft(void){
    if (x>0){
        x-=step_size_x;
        idx_x++;
    }
    _delay_ms(10);  //prevent weirdness   
}
/*******************************/
void crsr_right(void){
    if (x<X_RES){
        if (idx_x<8){  
            x+=step_size_x;           //allow two more grid squares past into hex edit
            //if(idx_x<7){idx_x--;}   //stop at end of bytebox
            idx_x--;
        } 
    }
    _delay_ms(10); //prevent weirdness
}

/*******************************/

void right_click(void){
    if(idx_y==0){ byte1 = byte1 ^ (1 << idx_x); }
    if(idx_y==1){ byte2 = byte2 ^ (1 << idx_x); } 
    if(idx_y==2){ byte3 = byte3 ^ (1 << idx_x); }         
    //if(idx_y==2){ byte3 = byte3 | (1 << idx_x); }  
    app_one_redraw();
}

/*******************************/
void left_click(void){
     byte3 = PINE;//TEST OF INPUTING DATA

     if(idx_y==0){ byte1 = byte1 &~(1 << idx_x); }
     if(idx_y==1){ byte2 = byte2 &~(1 << idx_x); }
     if(idx_y==2){ byte3 = byte3 &~(1 << idx_x); }

    app_one_redraw();
}

/*******************************/
void select_click(void){
    if(idx_y==0){ byte1 = 0xff; }
    if(idx_y==1){ byte2 = 0xff; }
    if(idx_y==2){ byte3 = 0xff; }
    app_one_redraw();
}

/*******************************/

void start_click(void){
    if(idx_y==0){ byte1 &=~0xff; }
    if(idx_y==1){ byte2 &=~0xff; }
    if(idx_y==2){ byte3 &=~0xff; }
    app_one_redraw();
}
/*******************************/

void draw_outline_box(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color){
   ST7735_LineRect(x  , y   ,w   , h  , grid_color);
   ST7735_FillRect(x+1, y+1 ,w-1 , h-1, color);
}

/*******************************/
void draw_byte_box(uint8_t byte, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t offcolor){
  /*
      draw a "byte" onscreen, represented as 8 squares with bits colored on or off 
      ARGS:
           byte     - buffer to hold the byte to be drawn 
           x,y      - corner to start drawing at, from top left 
           size     - scale factor for box (not much room on this small screen )
           color    - see adafruit lib for more info on colors 
  */  
    uint8_t num = 8;
    uint8_t a = 0;
    uint8_t total_w = x+w;
    uint8_t total_h = y+h;
    uint8_t bitsize = total_w/num;

    for (a=0;a<num;a++){
       if ( !!(byte & (1 << (7 - a))) ){  // MSB
           draw_outline_box(x+(a*bitsize), y, w/num, h, bit_on_color);
       }else{
           draw_outline_box(x+(a*bitsize), y, w/num, h, offcolor);
       }
    }

}

/*******************************/
void render_8_hex(uint8_t x, uint8_t y, uint8_t size, uint8_t accumulator, uint16_t color){
  /*
      draw a "byte" as a hex value, represented as 8 squares with bits colored on or off 
      ARGS:
           x,y         - corner to start drawing at, from top left 
           size        - scale factor for box (not much room on this small screen )
           accumulator - buffer to hold the byte to be drawn 
           color       - see adafruit lib for more info on colors 
  */

    uint8_t i = 0;

    uint8_t char_spacing = 6*size; //digit size in pixels
    uint8_t max = (char_spacing*2);//2 digits
    ST7735_FillRect(x, y, max+1, char_spacing+1, 0x0);    //clear rectangle in x*y space
    uint8_t cnt = 1;
    for (i=0;i<2;i++)
    {
        uint8_t digit = (accumulator & (0xf << (i*4))) >> (i*4);//little endian
        ST7735_DrawHexCharS((x+max)-(cnt*char_spacing), y+1, digit, color, 0x0, size);
        cnt++;
    }   
}

/*******************************/

void render_16_hex(uint8_t x, uint8_t y, uint8_t size, uint16_t accumulator, uint16_t color){
    

    uint8_t i = 0;

    uint8_t char_spacing = 6*size; //digit size in pixels
    uint8_t max = (char_spacing*4);//4 digits
    ST7735_FillRect(x, y, char_spacing*4, char_spacing, 0x0);    //clear rectangle in x*y space

    for (i=0; i<4; i++)
    {
        // Mask off one nibble at a time, shift it down to the
        // bottom of the integer, and use that as to index the font.
        // uint8_t digit = (accumulator & (0xf << (i*4))) >> (i*4);//little endian
        uint8_t digit = (accumulator & (0xf << (i*4))) >> (i*4);//little endian

        //view (nibble as working) as a graphic 
        //draw_byte_box(digit, 0,   1, byte_width, 20);

        //send (nibble as working) over serial port as human readble hex
        //USART_Transmit(hex_Font[digit]);//needs ascii encoding 

        //view (nibble as working) screen as human readable hex
        ST7735_DrawHexCharS((x+max)-(char_spacing*i), y, digit, color, 0x0, size);
     }   
}


/*******************************/
void draw_cursor(uint16_t color){
    ST7735_LineRect(x  , y   , byte_width/8    , byte_height   ,color);
    ST7735_LineRect(x+1, y+1 ,(byte_width/8)-2 , byte_height-2 ,color);   
    //ST7735_LineRect(x+2, y+2 ,(byte_width/8)-3 , byte_height-3 ,color);  
}

/*******************************/
//put here mostly because I forgot name of command
void clear_screen(uint16_t color){
    ST7735_FillScreen(color); 
}

/*******************************/

void bios_l_click(void){
    if(idx_y==0){ ST7735_FillScreen(0x00);app_one_redraw();cursor_loop();  }
    if(idx_y==1){ ST7735_FillScreen(0x00);sram_test1();   }
    //if(idx_y==2){ ST7735_FillScreen(0x00);sram_test2();   }
    if(idx_y==2){ ST7735_FillScreen(0x00);tiny_cpu();   }
    if(idx_y==3){ ST7735_FillScreen(0x00);sram_walk();    }
    if(idx_y==4){ ST7735_FillScreen(0x00);sram_erase();   }

    ST7735_FillScreen(0x00);
    unsigned char a[] = "*** DONE! ***";
    ST7735_DrawString(0, 50, a );   
}

/*************/

void boot_screen(){

    //ST7735_FillScreen(0x04af); //light blue bg
    //ST7735_FillRect(10, 10, 110, 150, 0x0099);  
    
    //DEBUG HOW DO YOU OVERWRITE A CHAR ARRAY??
    unsigned char a[] = "IO test";
    ST7735_DrawString(10,0, a );

    unsigned char b[] = "write SRAM";
    ST7735_DrawString(10,10, b );

    //unsigned char c[] = "write SRAM +10";
    //ST7735_DrawString(10,20, c );
    unsigned char c[] = "tiny cpu";
    ST7735_DrawString(10,20, c );

    unsigned char d[] = "walk SRAM";
    ST7735_DrawString(10,30, d );

    unsigned char e[] = "erase SRAM";
    ST7735_DrawString(10,40, e );

   // unsigned char f[] = "serial terminal";
   // ST7735_DrawString(10,50, f );


}


/*******************************/

void latch_outputs(void){
    PORTA = byte1;
    PORTC = byte2;
    //PORTC = byte4;    
}


/***********************************************/

/* COM buffer is for command args - Serial buffer is for stiching and tearing,
   helper to grab arguments for commands over serial port 
*/


 
uint16_t rx_two_bytes(void){
    uint8_t c1 =  USART_receive();//lsb addr
    uint8_t c2 =  USART_receive();//msb addr
    //send_txt_2bytes(buf1_16, 1, 0);
    scribe_str("2 bytes received.");
    return cvt_8x2_to_16(c1, c2); //fills buffer buf1_16

}


/***********************************************/

 //helper to grab arguments for commands over serial port 
 void rx_three_bytes(void){
    uint8_t c1 =  USART_receive();//lsb addr
    uint8_t c2 =  USART_receive();//msb addr 
    buf2_lsb =  USART_receive();//data byte 

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
    
    buf1_lsb =  USART_receive();//lsb addr
    buf1_msb =  USART_receive();//msb addr 
    buf2_lsb =  USART_receive();//lsb addr
    buf2_msb =  USART_receive();//msb addr 
    scribe_str("4 bytes received.");
}


/***********************************************/
//for printing internal 16 bit numbers - all serial related I/O is bytesX2
void send_txt_2bytes( uint16_t data, uint8_t use_newline,  uint8_t use_space){
   uint8_t i = 0;

   for (i=0; i<=15; i++) {
       //if (i==8){  USART_Transmit(0x20); }//middle space 

       if ( !!(data & (1 << (15 - i))) ){  // MSB
           USART_Transmit( BIT_OFF );
       }else{
           USART_Transmit( BIT_ON );
       }
    }
    
    if(use_space!=0){
        USART_Transmit(0x20);    //SPACE 
    }

    if(use_newline!=0){
        USART_Transmit( 0xa ); //CHAR_TERM = new line  
        USART_Transmit( 0xd ); //0xd = carriage return
    }
}

/***********************************************/
void halt_6502(void){

   //these should only be electrically active when 6502  is running 
   TOY_ADDR_MSBDDR = 0xff;     
   TOY_CTRL_DDR = 0x1f; ////bottom 5 bits as I/O
   TOY_ADDR_LSBDDR = 0xff;

   //send message to indicate we have halted
   if (terminal_mode){
        USARTWriteStr("System Halted."); 
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

   //LED_RGB_PORT=0x00; //clear all leds

   //send message to indicate we have finished initialization
   if (terminal_mode){
        USARTWriteStr("System Resume.");  
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
   Boundary scan like feature to get/set address and data buses 
*/

uint8_t get_dbus(void){
    TOY_DATA_DDR = 0x00; 
    uint8_t tmp = TOY_DATA_BUSIN;
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

/*******************************/
void test_up(void){
    //if(app_idx<2){
    //   app_idx==1;
    //}
    //select_click();
    //if (mem_addr<65535){mem_addr=mem_addr+1;}
    
    mem_addr++;

}

/*******************************/
void test_down(void){
    //if(app_idx>0){
    //    app_idx=2;
    //}
    //start_click();
    //if (mem_addr>0){mem_addr=mem_addr-1;}
    
    mem_addr--;
}


/*******************************/
void cursor_loop(void){
    while(1)
    {
        uint8_t nes_byte = nes_controller_read();
        //print_byte(nes_byte);
        if(nes_byte == 0xFE ) { crsr_right();  } //right
        if(nes_byte == 0xFD ) { crsr_lft();    } //left
        if(nes_byte == 0xFB ) { crsr_dwn();    } //down
        if(nes_byte == 0xF7 ) { crsr_up();     } //up
        if(nes_byte == 0xDF ) { test_down();    } //select
        if(nes_byte == 0xEF ) { test_up();      } //start
        if(nes_byte == 0x7F ) { right_click(); } //button a
        if(nes_byte == 0xBF ) { left_click();  } //button b
        if (lastx!=x || lasty!=y){
            app_one_redraw(); //redraw graphics
        }
        lastx = x;
        lasty = y;
    }
}


/*******************************/

void app_one_redraw(void){
    // ST7735_FillScreen(1);
    //////////////////////////////////
    //calculate values for boolean logic display   
    // uint8_t *pByte = 0;
    // pByte = &byte4;
    // or_bits(pByte, byte1, byte2);
    // pByte = &byte5;
    // and_bits(pByte, byte1, byte2);
    
    //////////////////////////////////

    draw_byte_box(byte1     , bb_begin        ,                0,  byte_width , byte_height , bit_off_color);
    render_8_hex(hex_begin  , 0               , fontsize   ,  byte1      , CRSR_COLOR);
    //--------  
    draw_byte_box(byte2     , bb_begin        , byte_height , byte_width  , byte_height , bit_off_color);
    render_8_hex(hex_begin  , byte_height     , fontsize  , byte2       , CRSR_COLOR); 
    //--------  
    draw_byte_box(byte3     , bb_begin        , (byte_height*2) , byte_width  , byte_height , bit_off_color);
    render_8_hex(hex_begin  , (byte_height*2) , fontsize  , byte3       , CRSR_COLOR); 
    //--------
    // draw_byte_box(byte4     , bb_begin        , (byte_height*3) , byte_width  , byte_height , or_off_color);
    // render_8_hex(hex_begin  , (byte_height*3) , fontsize  , byte4       , CRSR_COLOR);
    // //--------
    // draw_byte_box(byte5     , bb_begin        , (byte_height*4) , byte_width  , byte_height , and_off_color);
    // render_8_hex(hex_begin  , (byte_height*4) , fontsize  , byte5       , CRSR_COLOR);
    
    //--------    
    //--------
    draw_cursor(CRSR_COLOR); //draw new cursor

    //show the memory address that we are navigating  
    //render_16_hex(0 , 100 , 2  , mem_addr    , CRSR_COLOR);
    //ST7735_DrawCharS(0 , 100 , mem_addr,    CRSR_COLOR, 0x00, 2);//16 NOT 8 bit debug

    //show the memory address that we are navigating  
    //render_8_hex(0  , 120 , 2  , byte3       , CRSR_COLOR);
    ST7735_DrawCharS(0 , 100 , mem_addr,    CRSR_COLOR, 0x00, 2);

    //--------
    latch_outputs();//set the ports to reflect the changes
}

/*******************************/
/*
write sequential data to ram and read it back out 
*/
void sram_test1(void){
    unsigned char foo[] = "Testing SRAM";
    ST7735_DrawString(0,90, foo );
    //------
    int a = 0; 
    for(a=0;a<256;a++){
        write_ram(a, a);
        PORTE = 0x00;//stupid hack - debug
        render_8_hex(0  , 100 , 2  , read_ram(a)      , CRSR_COLOR);        
       //_delay_ms(100);
    }
}

/*******************************/
void sram_test2(void){
    //iterate SRAM, add 10 to the address value and write that to memory
    unsigned char foo[] = "Testing SRAM";
    ST7735_DrawString(0,90, foo );
    //------
    int a = 0; 
    for(a=0;a<240;a++){
        write_ram(a, a+10);
        PORTE = 0x00;//stupid hack - debug
        render_8_hex(0  , 100 , 2  , read_ram(a)      , CRSR_COLOR);        
       //_delay_ms(100);
    }
}


/*******************************/
void tiny_cpu(void){
    unsigned char foo[] = "Tiny CPU";
    ST7735_DrawString(10,0, foo );
    //------
    while(1){
        uint8_t nez_byte = nes_controller_read();    
        render_8_hex(0  , 20 , 2  , nez_byte      , CRSR_COLOR);        
    }
    _delay_ms(10);
} 

/*******************************/
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

/*******************************/
//read back ram and DRAW BYTEBOX!!
void sram_walk(void){
    uint16_t a = 0; 
    ST7735_FillScreen(0x00); //clear all black 

    for(a=0;a<1024;a++){
        PORTE = 0x00;//stupid hack - debug

        draw_byte_box(PORTA     , bb_begin        ,            0,  byte_width , byte_height , bit_off_color);
        render_8_hex(hex_begin  , 0               , fontsize    ,  PORTA      , CRSR_COLOR);
        //--------  
        draw_byte_box(PORTC     , bb_begin        , byte_height , byte_width  , byte_height , bit_off_color);
        render_8_hex(hex_begin  , byte_height     , fontsize    , PORTC       , CRSR_COLOR); 
        //--------  
        draw_byte_box(read_ram(a)     , bb_begin        , (byte_height*2) , byte_width  , byte_height , bit_off_color);
        render_8_hex(hex_begin  , (byte_height*2) , fontsize  , read_ram(a)       , CRSR_COLOR); 
        
        //--------  
        //draw_byte_box(PORTF    , bb_begin        , (byte_height*3) , byte_width  , byte_height , bit_off_color);
        //render_8_hex(hex_begin  , (byte_height*3) , fontsize  , PORTF            , CRSR_COLOR); 

        render_16_hex(0  , 50 , 2  , a                , 0xff00);  //ADDRESS IN HEX       
        //render_8_hex(75  , 30 , 2  , read_ram(a)      , CRSR_COLOR);        
       _delay_ms(500);
    }
}


/***********************************************/

void listen_serial(void)
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
                        //reset_6502(); //not working right 
                        isknown=1;            
                    break;

                    
                    case CMD_NES_DRAW: //n
                        //testNES2(); 
                    break;

                   
                    case CMD_CLR_SCRN: //N
                           ST7735_FillScreen(0); //clear screen black ; 
                           cursor_y = 0;
                           cursor_x = 0;              
                    break;
                    
                    
                    case CMD_BITMAP: //B
                        //TEST - SEND START AND END LOCATIONS OVER IN COMPRESSED 16 BIT VALUE  
                        st_px = (uint8_t)(sprite_xy >> 8);     //LSB of address
                        ed_px = (uint8_t)(sprite_xy & 0x00FF); //MSB of address

                        //ST7735_DrawBitmapSRAM(st_px, ed_px, 0, 10, 10);
                    break;

                    
                    case CMD_BITMAP2: //C
                        //blit_test();
                    break;           


                    
                    case CMD_BITMAP3: //D
                        //bitmap_test();
                    break; 
                             

                   

                    case CMD_SHOWINTERNALS: //s  - show globals
                        isknown=1;            
                        
                        //cvt_8x2_to_16(buf1_lsb, buf1_msb);
                        send_txt_2bytes( buf2_16, 0, 1);
                        //cvt_8x2_to_16(buf2_lsb, buf2_msb );
                        send_txt_2bytes( buf1_16, 1, 1);
                        // uint8_t ishalted = 1;  
                    break;
                   

                    //terminal_mode = 1; //TOGGLE TERMINAL MODE - RUN IN SINGLE TERMINAL

                    //case CMD_TOGGLE_TERMODE:
                    //break;

                  
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

                   
                    case CMD_ECHO: //q
                        _delay_us(300);
                   
                        echo_uart();
                        USART_Transmit( 0x0d ); // CR          
                        USART_Transmit( 0x0a );
                        isknown=1;
                    break;

                 

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
                        // USART_Transmit( 0x42 );           
                        // USART_Transmit( 0x0d ); // CR          
                        // USART_Transmit( 0x0a );            
                        isknown=1;

                    break;

                    case CMD_PEEK_6502: //p 
                        //peek a 16 bit location from 2 bytes 
                        buf1_16 = rx_two_bytes(); 
                     
                        USART_Transmit(0x61);                          // a
                        USART_Transmit(0x20);                          //space
                        send_txt_byte( (uint8_t)(buf1_16 >> 8) , 0);     //LSB of address
                        send_txt_byte( (uint8_t)(buf1_16 & 0x00FF), 0);  //MSB of address
                        USART_Transmit(0x20);                          //space      
                        USART_Transmit(0x64);                          // d
                        USART_Transmit(0x20);                          //space 
                        send_txt_byte( read_ram(buf1_16), 1);            // data byte
                        isknown=1; 

                        //scribe_str(".........")
                        //scribe_byte2_astext(buf1_16);            //SHOW ADDR 
                        //scribe_byte_astext( read_ram(buf1_16) ); //SHOW DATA

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


                    //pretty sure this isnt the way to properly do this  
                    case CMD_RESUM_6502: //c  
                        resume_6502();
                        isknown=1;
                        leavered=0; 
                        
                        scribe_str("clocking 6502 CPU");
                        //you can use external clock and not lock the bugger up 
                        //spinclock();//run clock forever  

                    break;

                    //pretty sure this isnt the way to properly do this           
                    case CMD_HALT_6502: //h  
                        scribe_str("halt");
                        halt_6502();
                        isknown=1; 
                        leavered=1; 
                    break;

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

                    // UNTESTED!
                    case CMD_TESTSRAM: //t  - flood ram index%256
                        isknown=1;  
                        flood_pattern();           
                    break;


                }
        }//else (not halted)

    }//while    

}


