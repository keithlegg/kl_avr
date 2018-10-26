#include <avr/io.h>

#define F_CPU 16000000UL //AVR Clock Speed in MHZ
#define FOSC 16000000    // Clock Speed

#include <util/delay.h>

/********************/

//allow for remapping of LEDs 
#define a_deg 0x80
#define b_deg 0x40
#define c_deg 0x20
#define d_deg 0x10
#define e_deg 0x08
#define f_deg 0x04
#define g_deg 0x02
#define dp_deg 0x01



/********************/

#define del 1      //small delay for slow human eyes ( POV brightness adjust)
#define lngdel 100 //long delay for slow human brains 

uint8_t dgt_idx = 0; //DIGIT is a NUMBER OR LETTER
//uint8_t seg_idx = 0; //SEGMENT IS A PORTION OF A DIGIT 


/********************/

//   7-Segment display hexadecimal font. High bit is 'a'
//        a
//      f   b
//        g
//      e   c
//        d    h
static int s_Font[16] = {
        0b11111100,             // 0
        0b01100000,             // 1
        0b11011010,             // 2
        0b11110010,             // 3
        0b01100110,             // 4
        0b10110110,             // 5
        0b10111110,             // 6
        0b11100000,             // 7
        0b11111110,             // 8
        0b11100110,             // 9
        0b11101110,             // A
        0b00111110,             // b
        0b00011010,             // c
        0b01111010,             // d
        0b10011110,             // E
        0b10001110              // F
};

/********************/
void map_character(uint8_t charbyte){

    if (charbyte=='off'){
        PORTD = 0x00;    
        //no delay needed for off state    
    }

    if (charbyte=='0'||charbyte==0){
        PORTD = s_Font[0]; 
        _delay_ms(del);     
    }

    if (charbyte=='1'||charbyte==1){
        PORTD = s_Font[1]; 
        _delay_ms(del);         
    }

    if (charbyte=='2'||charbyte==2){
        PORTD = s_Font[2]; 
        _delay_ms(del);                
    }

    if (charbyte=='3'||charbyte==3){
        PORTD = s_Font[3];   
        _delay_ms(del);             
    }

    if (charbyte=='4'||charbyte==4){
        PORTD = s_Font[4]; 
        _delay_ms(del);         
    }

    if (charbyte=='5'||charbyte==5){
        PORTD = s_Font[5]; 
        _delay_ms(del);                 
    }

    if (charbyte=='6'||charbyte==6){
        PORTD = s_Font[6]; 
        _delay_ms(del);                
    }

    if (charbyte=='7'||charbyte==7){
        PORTD = s_Font[7]; 
        _delay_ms(del);         
    }

    if (charbyte=='8'||charbyte==8){
        PORTD = s_Font[8]; 
        _delay_ms(del);          
    }

    if (charbyte=='9'||charbyte==9){
        PORTD = s_Font[9]; 
        _delay_ms(del);          
    }
                
    if (charbyte=='a'||charbyte==10){
        PORTD = s_Font[10]; 
        _delay_ms(del);                
    }

    if (charbyte=='b'||charbyte==11){
        PORTD = s_Font[11]; 
        _delay_ms(del);         
    }

    if (charbyte=='c'||charbyte==12){
        PORTD = s_Font[12]; 
        _delay_ms(del);         
    }

    if (charbyte=='d'||charbyte==13){
        PORTD = s_Font[13];  
        _delay_ms(del);                
    }

    if (charbyte=='e'||charbyte==14){
        PORTD = s_Font[14]; 
        _delay_ms(del);                
    }

    if (charbyte=='f'||charbyte==15){
        PORTD = s_Font[15]; 
        _delay_ms(del);                
    }

    /*******************/

}


/********************/
//map an integer to a byte masked single bit  
uint8_t idx_to_byte(uint8_t idx){
    return (1 << idx);
} 

/********************/
/*
void clear_digit(void){
    map_character('off');
}
*/

/********************/
//auto increment which digit is on, wrap if at end
void incr_digit(void){
    if(dgt_idx>=3){   
        dgt_idx=3;
    }
    if(dgt_idx<3){dgt_idx++;}
    PORTB = (1 << dgt_idx);
}

/********************/
//auto decrement which digit is on, wrap if at end
void decr_digit(void){
    if (dgt_idx<=0){
        dgt_idx=0; 
    }
    if(dgt_idx>0){dgt_idx--;}
    PORTB = (1 << dgt_idx);
}
/********************/

/*
void bin2bcd( unsigned int val )
{
  char i;

  i = '0' - 1;
  do
    i++;
  while( !((val -= 10000) & 0x8000) );
  digit[4] = i;

  i = '0' + 10;
  do
    i--;
  while( (val += 1000) & 0x8000 );
  digit[3] = i;

  i = '0' - 1;
  do
    i++;
  while( !((val -= 100) & 0x8000) );
  digit[2] = i;

  i = '0' + 10;
  do
    i--;
  while( (val += 10) & 0x8000 );
  digit[1] = i;

  digit[0] = val | '0';
}
*/

/*
void Short2BCD(unsigned short in, unsigned char *digit)
// returns BCD-converted value. Max 5 digits
{
    unsigned short val;
    unsigned char i;
    for (i=0;i<=4;i++)
    {
      val = in % 10;
      //digit[i]=val; //add value of '0' for ASCII
      in /= 10;
    }
}
*/
/********************/
void map_numeric(uint16_t charbyte){

   uint8_t ones_digit  = 0;
   uint8_t tens_digit  = 0;
   uint8_t hndr_digit  = 0;
   uint8_t thos_digit  = 0;

   unsigned short val;
   unsigned char i;

   dgt_idx = 0;

    for (i=0;i<=4;i++)
    {
      val = charbyte % 10;

      if(i==0){ones_digit = val;}
      if(i==1){tens_digit = val;}
      if(i==2){hndr_digit = val;}
      if(i==3){thos_digit = val;}                 
      charbyte /= 10;
    }

   //----------  
   PORTB = 0x01;
   map_character(ones_digit);
   _delay_ms(1);
   PORTD = 0x00; 
   //incr_digit();
   
   //----------
   PORTB = 0x02;
   map_character(tens_digit);
   _delay_ms(1);   
   PORTD = 0x00; 
   //incr_digit();
   //---------- 
   PORTB = 0x04; 
   map_character(hndr_digit);
   _delay_ms(1);   
   PORTD = 0x00; 
   //incr_digit();
 
   //---------- 
   PORTB = 0x08;
   map_character(thos_digit);
   _delay_ms(1);   
   PORTD = 0x00; 
        
}



/********************/
/********************/
/********************/
//test function - flash all digits left to right
void test_cyclon(void)
{
    int a = 0;
    int b = 0;

    PORTD = 0xff; //turn on all segments 

    dgt_idx=0;

    for(a=0;a<4;a++){
        
         for(b=0;b<16;b++){
            map_character(b); 
            _delay_ms(lngdel);  
            PORTD = 0x00;
         }
         incr_digit();
     }

    for(a=0;a<4;a++){
        
         for(b=0;b<16;b++){
            map_character(b); 
            _delay_ms(lngdel);  
            PORTD = 0x00;
         }
         decr_digit();
     }

}

/********************/
//test to see all bits of display flash
void test_cycle_all(void)
{
    int s = 0;
    int a = 0;

    for(s=0;s<4;s++)
    {
        for(a=7;a>=0;a--){
            PORTD = idx_to_byte(a);
            _delay_ms(lngdel);
        }
        for(a=0;a<7;a++){
            PORTD = idx_to_byte(a);
            _delay_ms(lngdel);
        }

        incr_digit();

    }
    PORTB = 0x01;    
    dgt_idx=0;

}

/********************/
void test_count(void){
   int a = 0;  

   for(a=0;a<10000;a++){
       map_numeric(a);
  }
}



/********************/
/********************/

int main (void)
{
   //DDRB =0b00001000;  //arduino pin 11 
   //DDRB |= (1 << 3);  //arduino pin 11

   DDRB = 0x0f; //digit select 0-4 ,PB5 is input for count    
   DDRD = 0xff; //segment select  
  
   PORTB = 0x01;

   while (1)
   {

       //test_cyclon();
       test_count();

   }

} 

/////////////////



