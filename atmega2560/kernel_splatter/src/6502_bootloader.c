#include <avr/io.h>

#include <avr/interrupt.h> //DONT INCLUDE UNLESS YOU NEED

#define F_CPU 16000000UL //AVR Clock Speed in MHZ
#define FOSC 16000000UL    // Clock Speed

//UART STUFF
#define BAUD 57600
#define MYUBRR FOSC/16/BAUD-1

#define BIT_ON 0x30 //logic high
#define BIT_OFF 0x31 //logic low

#include <util/delay.h>


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

//PORTD - CONTROL BUS - bits 1-3

//                          SRAM   | 6502     | AVR 
//------------------------------------------------   
#define WE_PIN 0x01       //29  WE | 34 R/W   | PD0
#define CLK_PIN 0x02      //       | 37 phi0  | PD1
#define SRAM_CE 0x04      //30 CE2 | 39 phi2  | PD2
#define MOS6502_RST 0x08  //       | 40 reset | PD3  

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
#define CMD_SET_INTERNALS 0x53     //S - set  global numeric buffer  

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


/***********************************************/

//STATES FOR MACHINE BIOS 
uint8_t ishalted = 1;       //start out halted

//in NON terminal mode - send ACK after each command?
uint8_t terminal_mode = 1;  //be more chatty over serial for command feedback to humans (instead of python)


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
/*THESE USE GLOBALS INSTEAD OF RETURNING A VALUE!!!*/

void cvt_8x2_to_16(uint8_t msbin, uint8_t lsbin){
   
   //combine two bytes into a 16 - output goes to global buffer
   uint8_t dat_8;
   uint16_t dat_16;
   dat_8 = msbin;
   dat_16 = (lsbin<<8);
   buf1_16 = (dat_16 | dat_8);
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

/***********************************************/
/***********************************************/
/*
   send a byte over serial port represented as text - 1's and 0's 
*/

void print_byte( uint8_t data, uint8_t use_newline){
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
void print_2bytez( uint16_t data, uint8_t use_newline,  uint8_t use_space){
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
//walk each bit and flash for testing wiring, etc 
void test_addr_bits(void){
    int a=0;
    for(a=0;a<8;a++){
       PORTL = idx_to_byte(a);
       _delay_ms(100);
    }
    PORTL = 0x00;
    for(a=0;a<8;a++){

       PORTC = idx_to_byte(a);
       _delay_ms(100);
    }       
    PORTC=0x00;
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
    
    PORTL = low_byte;
    PORTC = high_byte;
}


/***********************************************/
/*
   read a location in SRAM 
*/

uint8_t read_ram(uint16_t address){
    
    uint8_t out = 0x00;

    DDRK = 0x00; 
    PORTK = 0x00; //clear data port
    set_address(address);
    PORTD |= WE_PIN; //WE high when reading 
    out = PINK;
    PORTK = 0x00; //clear data port
    return out;
}

/***********************************************/
/*
  write to a location in SRAM 
*/

void write_ram(uint16_t address, uint8_t byte){
    
    DDRK = 0xff;
    PORTD |= WE_PIN; 
    set_address(address);
    PORTK = byte;
    PORTD &= ~WE_PIN;     
    PORTD |= WE_PIN;  
}



/*
//THOUGHT THESE WORK - NOW NOT SURE OF ANYFUCKING THING 

uint8_t read_ram(uint16_t address){
    uint8_t out = 0x00;
    DDRK = 0x00; 
    set_address(address);
    PORTD |= WE_PIN; //WE high when reading 
    out = PINK;
    return out;
}
//THOUGHT THESE WORK - NOW NOT SURE OF ANYFUCKING THING 
  
void write_ram(uint16_t address, uint8_t byte){
    
    DDRK = 0xff;
    PORTD |= WE_PIN; 
    set_address(address);
    PORTK = byte;
    PORTD &= ~WE_PIN;     
    PORTD |= WE_PIN;  
}
*/


/***********************************************/
/*
   Boundary scan like feature to get/set address and data buses 
*/

uint8_t get_dbus(void){
    DDRK = 0x00; 
    uint8_t tmp = PINK;
    DDRK = 0xff; 
    return tmp;
}

/***********************************************/
/*
   Boundary scan like feature to get/set address and data buses 
*/

void set_dbus(uint8_t byte){
    DDRK = 0xff; 
    PORTK = byte;
    //DDRK = 0x00; 
}

/***********************************************/
/*
   Boundary scan like feature to get/set address and data buses 
*/

uint16_t get_abus(void){
    cvt_8x2_to_16(PORTC, PORTL);
    return buf1_16;
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
             PORTH^=0x02;//red 
         } 
         if(b%2048==0){
             PORTH^=0x01; //green  
         } 
         
     }//B-Y
}

/***********************************************/
void echo_uart(void){
    uint8_t buf = UART_receive();
    print_byte(buf, 1);
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
          print_byte( (uint8_t)(a >> 8) , 0);//LSB of address
          print_byte( (uint8_t)(a & 0x00FF), 0);//MSB of address
          UART_Transmit(0x20); //space      
          UART_Transmit(0x64); // d
          UART_Transmit(0x20); //space
          print_byte( read_ram(a), 1);//data
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
   PORTH = 0x00;  //clear LEDS
   PORTH |= 0x02; //turn on green - (setting the first spinlocked byte as RED)
   
   //clamp at 65535
   for(a=0;a<numbytes;a++)
   { 
      
      //flash red to indicate we are working on loading data 
      if(a%2048==0){
          PORTH^=0x01; PORTH^=0x02; //toggle both red and green   
      } 
 
       write_ram(a, UART_receive() ); //pack 16 bit value into 1 byte to send 
   }
  
   PORTH=0x02; 
}

/***********************************************/
void flood_sram(uint8_t byte){
   uint16_t a = 0;   
   for(a=0;a<65535;a++){ 
      if(a%16384==0){
          PORTH^=0x01;PORTH^=0x02;   
      } 
      write_ram(a, byte ); 
   } 
}

/***********************************************/
void clear_sram(void){
    //send message to indicate we have finished initialization
    if (terminal_mode){
        UART_Transmit(0x69);// i
        UART_Transmit(0x6e);// n
        UART_Transmit(0x69);// i
        UART_Transmit(0x74);// t
        UART_Transmit(0x69);// i
        UART_Transmit(0x61);// a
        UART_Transmit(0x6c);// l
        UART_Transmit(0x69);// i
        UART_Transmit(0x7a);// z
        UART_Transmit(0x69);// i
        UART_Transmit(0x6e);// n
        UART_Transmit(0x67);// g
        UART_Transmit(0x20);// space
        UART_Transmit(0x53);// S
        UART_Transmit(0x52);// R
        UART_Transmit(0x41);// A
        UART_Transmit(0x4d);// M
        UART_Transmit(0xa); // \n  
        UART_Transmit(0xd); // \CR  
    }

   //clear entire memory space 
   flood_sram(0x00);
}
/***********************************************/
void init_device(void){
   //activate registers to access SRAM and LEDs
   // be sure to set back to HI-Z when starting 6502 up!
   DDRC = 0xff;     
   DDRD = 0x07;  
   DDRL = 0xff; 
   //DDRK IS SET DYNAMICALLY 

   DDRH = 0x03; //red and green LEDs 0x02 green, 0x01 red

   //setup serial port communication
   UART_Init(MYUBRR);

   //SET INITIAL PIN STATES FOR CONTROL BUS 
   PORTD |= CLK_PIN; //ram only can be written to when this is high (phi2 pin is inverted) 
   PORTD |= SRAM_CE; //high on sram pin 30 is wirte to RAM 



   //DEBUG - NORMALLY YOU WANT THIS, BUT I ADDED BATTERY BACKUP SO I WANT TO TEST THAT IT KEEPS
   //clear_sram();
   //DEBUG - NORMALLY YOU WANT THIS, BUT I ADDED BATTERY BACKUP SO I WANT TO TEST THAT IT KEEPS



   if (terminal_mode){
       UART_Transmit(0x72);// r
       UART_Transmit(0x65);// e
       UART_Transmit(0x61);// a
       UART_Transmit(0x64);// d
       UART_Transmit(0x79);// y
       UART_Transmit(0xa); // \n  
       UART_Transmit(0xd); // \CR    
   }
   PORTH =0x01; //red led  
}

/***********************************************/

/* COM buffer is for command args - Serial buffer is for stiching and tearing,
   helper to grab arguments for commands over serial port 
*/
void rx_two_bytes(void){
    uint8_t c1 =  UART_receive();//lsb addr
    uint8_t c2 =  UART_receive();//msb addr
    cvt_8x2_to_16(c1, c2); //fills buffer buf1_16
    //print_2bytez(buf1_16, 1, 0);
}
/***********************************************/
 
 //helper to grab arguments for commands over serial port 
 void rx_three_bytes(void){
    uint8_t c1 =  UART_receive();//lsb addr
    uint8_t c2 =  UART_receive();//msb addr 
    buf2_lsb =  UART_receive();//data byte 

    cvt_8x2_to_16(c1, c2); //fills buffer buf1_16

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
    // print_2bytez(buf1_16, 1, 0);
    // print_2bytez(buf2_16, 1, 0);
    
    buf1_lsb =  UART_receive();//lsb addr
    buf1_msb =  UART_receive();//msb addr 
    buf2_lsb =  UART_receive();//lsb addr
    buf2_msb =  UART_receive();//msb addr 

}


/***********************************************/
void halt_6502(void){

   //these should only be electrically active when 6502  is running 
   DDRC = 0xff;     
   DDRD = 0x1f; ////bottom 5 bits as I/O
   DDRL = 0xff;

   //send message to indicate we have halted
   if (terminal_mode){
       UART_Transmit(0x53);// S
       UART_Transmit(0x79);// y
       UART_Transmit(0x73);// s      
       UART_Transmit(0x74);// t
       UART_Transmit(0x65);// e
       UART_Transmit(0x6d);// m
       UART_Transmit(0x20);// space
       UART_Transmit(0x68);// h
       UART_Transmit(0x61);// a
       UART_Transmit(0x6c);// l
       UART_Transmit(0x74);// t
       UART_Transmit(0xa); // \n  
       UART_Transmit(0xd); // \CR   
   } 
   ishalted =1;
}

//resume is start - same thing
void resume_6502(void){
   
   //set all AVR pins to HIZ to avoid bus contention 
   DDRC = 0x00;     
   DDRL = 0x00;
   DDRK = 0x00;
   DDRD = 0xf6; // 11110110 , (all but reset pin runs but no memory write)

   PORTH=0x00; //clear all leds

   //send message to indicate we have finished initialization
   if (terminal_mode){
       UART_Transmit(0x53);// S
       UART_Transmit(0x79);// y
       UART_Transmit(0x73);// s      
       UART_Transmit(0x74);// t
       UART_Transmit(0x65);// e
       UART_Transmit(0x6d);// m
       UART_Transmit(0x20);// space
       UART_Transmit(0x72);// r
       UART_Transmit(0x65);// e
       UART_Transmit(0x73);// s   
       UART_Transmit(0x75);// u
       UART_Transmit(0x6d);// m
       UART_Transmit(0x65);// e
       UART_Transmit(0xa); // \n  
       UART_Transmit(0xd); // \CR    
   }
   //PORTD |= 0x08; //reset high (active low) 

    /* USE A TIMER INTERRUPT INSTEAD SO IT WONT LOCK UP
    while(1){
        PORTH=0x00;
        _delay_ms(400);
        PORTH=0x02;
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
        print_byte( read_ram(y) ,1);
    }    
}

/***********************************************/
void reset_6502(void){
    //THIS ONLY PAUSES , NOT RESETS?? I DONT KNOW?
    //maybe AVR cant pull the line low enough since it is pulled up via resistor?

    //set PORTD bit 4 to OUTPUT 
    DDRD |= MOS6502_RST;
    PORTD &= ~MOS6502_RST;//set low

    //IF YOU DONT PULSE CLOCK IT JUST PAUSES
    PORTD ^= CLK_PIN;
    PORTD ^= CLK_PIN;
    PORTD ^= CLK_PIN;
    PORTD ^= CLK_PIN;
    PORTD ^= CLK_PIN;
    PORTD ^= CLK_PIN;

    DDRD &=~MOS6502_RST;//set back to run 
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
       PORTD ^= CLK_PIN;
       //_delay_us(2); //1 Mhz clock
       //if(led_cnt<100){PORTH = 0x02;}//blink led @ 1Mhz
       _delay_ms(10);//much slower than 1 Mhz!  
       if(led_cnt>100){PORTH = 0x02;led_cnt=0; }//blink led @ slow speed  
       else {PORTH=0x00;led_cnt++;}              //blink led @ slow speed            

    }
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
 
//Writing this bit to one enables interrupt on the RXCn Flag. A USART Receive Complete interrupt will be generated
//only if the RXCIEn bit is written to one, the Global Interrupt Flag in SREG is written to one and the RXCn bit in
//UCSRnA is set.

void init_uart_interrupt(){
   UCSR0B |= (1 << RXCIE0); // Enable the USART Recieve Complete interrupt (USART_RXC) 
   //sei(); // Enable the Global Interrupt Enable flag so that interrupts can be processed 
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
       if(1){ //USE ABOVE CODE LATER FOR HALT CONDITIONS
                switch(UDR0)
                {

                    case CMD_RST_6502: //r  - reset 6502
                        reset_6502(); //not working right 
                        isknown=1;            
                    break;

                    /**********/

                    case CMD_SHOWINTERNALS: //s  - show globals
                        isknown=1;            
                        
                        //cvt_8x2_to_16(buf1_lsb, buf1_msb);
                        print_2bytez( buf2_16, 0, 1);
                        //cvt_8x2_to_16(buf2_lsb, buf2_msb );
                        print_2bytez( buf1_16, 1, 1);
                        // uint8_t ishalted = 1;  
                    break;
                    /**********/

                    //terminal_mode = 1; //TOGGLE TERMINAL MODE - RUN IN SINGLE TERMINAL

                    //case CMD_TOGGLE_TERMODE:
                    //break;

                    /**********/

                    case CMD_SET_INTERNALS: //S  - set globals
                        isknown=1;            
                        rx_four_bytes(); 
             
                        cvt_8x2_to_16(buf1_lsb, buf1_msb);
                        buf2_16 = buf1_16;
                        cvt_8x2_to_16(buf2_lsb, buf2_msb );
                    break;

                    /**********/
                    case CMD_SET_ADDR_BUS: //a
                        rx_two_bytes();
                        print_2bytez(buf1_16, 1, 0);
                        set_abus(buf1_16);
                    break;
                    
                    case CMD_GET_ADDR_BUS: //b  
                        //print_2bytez(buf1_16, 0 ,0);
                    break;
                    
                    case CMD_SET_DATA_BUS: //e  
                        //set_dbus(UART_receive());
                    break;
                    
                    case CMD_GET_DATA_BUS: //f 
                        print_byte(get_dbus(), 1);
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
                        rx_ram(65535);
                        
                        //show_ram_region(65300, 65400);
                        //_delay_ms(100);
                        //resume_6502();
                        
                        isknown=1;               
                    break;

                    //download SRAM "disk image" (6502->compy)
                    case CMD_TX_SRAM: //d  
                        _delay_ms(100); 
                        tx_bytes(0, 65535);
                        // UART_Transmit( 0x42 );           
                        // UART_Transmit( 0x0d ); // CR          
                        // UART_Transmit( 0x0a );            
                        isknown=1;
                    break;

                    case CMD_PEEK_6502: //p 
                        //peek a 16 bit location from 2 bytes 
                        rx_two_bytes();
                     
                        UART_Transmit(0x61);                          // a
                        UART_Transmit(0x20);                          //space
                        print_byte( (uint8_t)(buf1_16 >> 8) , 0);     //LSB of address
                        print_byte( (uint8_t)(buf1_16 & 0x00FF), 0);  //MSB of address
                        UART_Transmit(0x20);                          //space      
                        UART_Transmit(0x64);                          // d
                        UART_Transmit(0x20);                          //space 
                        print_byte( read_ram(buf1_16), 1);            // data byte
                        isknown=1;            
                    break;

                    case CMD_PEEKRANGE: //P  - NOT DONE
                        //peek a range of addresses - 2 bytes start , 2 bytes stop 
                        isknown=1; 
                        rx_four_bytes(); 
             
                        cvt_8x2_to_16(buf1_lsb, buf1_msb);
                        buf2_16 = buf1_16;
                        cvt_8x2_to_16(buf2_lsb, buf2_msb );
                        show_ram_region(buf2_16, buf1_16);
                    break;

                    //PEEK AND POKE ARE UNTESTED!
                    case CMD_POKE_6502:     //o - set memory (3X8bit args)  
                        rx_three_bytes();
                        write_ram(buf1_16, buf2_lsb);// buf1_16= addr  buf2_lsb = data
                        
                        //print_2bytez(buf1_16, 0, 0) ;      //sends 19 bytes with both set to 1
                        //print_byte( read_ram(buf1_16), 0 ); 

                        isknown=1;            
                    break;

                    /**************************************/
                    //pretty sure this isnt the way to properly do this  
                    case CMD_RESUM_6502: //c  
                        resume_6502();
                        isknown=1;
                        leavered=0; 
                        //you can use external clock and not lock the bugger up 
                        spinclock();//run clock forever  
                    break;

                    //pretty sure this isnt the way to properly do this           
                    case CMD_HALT_6502: //h  
                        halt_6502();
                        isknown=1; 
                        leavered=1; 
                    break;

                    /**********/
                    /**************************************/
                    /**********/

                    case CMD_FREERUN: //z  - flood ram with NOPs
                        isknown=1;
                        flood_sram(0xea);//NOP byte 0xea           
                    break;

                    case CMD_ZEROSRAM: //x  - flood ram with 0's
                        isknown=1;  
                        clear_sram();          
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
            PORTH=0x01;
        }else if(leavegreen){
            PORTH=0x02;
        }else{
          //heartbeat
          PORTH = 0x00;
          if(isknown){PORTH|=0x02;}else{PORTH|=0x01;}  
          _delay_ms(20);
          PORTH =0x00; 
        }

    }//while    

}

/***********************************************/
int main (void)
{
    init_device();
    //init_timer_interrupts();//use timer interrupt to clock 6502
    //init_uart_interrupt();//test of stopping a running machine 
    listen_serial();


} 

/***********************************************/

//interrupts work well for clock because you can stop,
// but it fuqs up all the timing. They load AVRs stack and add spikes in the timing

/*
ISR ( TIMER1_COMPA_vect )
{
    if(ishalted){
        //PORTD =0x00;
    }else{
        PORTD ^= CLK_PIN; 
    }
}*/

/***********************************************/

/*
ISR( USART0_RX_vect)
{
    char ReceivedByte;
    ReceivedByte = UDR0; 

    //?? When interrupt-driven data reception is used, the receive complete routine must read the 
    //?? received data from UDR in order to clear the RXC Flag, otherwise a new interrupt will occur 
    //?? once the interrupt routine terminates.
    if (ReceivedByte==0x68){
        // Code to be executed when the USART receives a byte here
        PORTH ^= 0x01;
        ishalted=1;  
        cli();//immediately turn off interrupts when halted      
    }
}
*/ 

