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
#define lngdel 1 //long delay for slow human brains 

uint8_t dgt_idx = 0; //DIGIT is a NUMBER OR LETTER
uint8_t seg_idx = 0; //SEGMENT IS A PORTION OF A DIGIT 


/********************/
void map_character(uint8_t charbyte){

    if (charbyte=='off'){
        PORTD = 0x00;    
        //no delay needed for off state    
    }

    if (charbyte=='0'||charbyte==0){
        PORTD |= a_deg;
        PORTD |= b_deg;
        PORTD |= c_deg;
        PORTD |= d_deg;
        PORTD |= e_deg;
        PORTD |= f_deg;  
        _delay_ms(del);     
    }

    if (charbyte=='1'||charbyte==1){
        PORTD |= b_deg;
        PORTD |= c_deg;
        _delay_ms(del);         
    }

    if (charbyte=='2'||charbyte==2){
        PORTD |= a_deg;
        PORTD |= b_deg;
        PORTD |= g_deg;
        PORTD |= e_deg;
        PORTD |= d_deg; 
        _delay_ms(del);                
    }

    if (charbyte=='3'||charbyte==3){
        PORTD |= a_deg;
        PORTD |= b_deg;
        PORTD |= c_deg;
        PORTD |= d_deg;
        PORTD |= g_deg;    
        _delay_ms(del);             
    }

    if (charbyte=='4'||charbyte==4){
        PORTD |= b_deg;
        PORTD |= c_deg;
        PORTD |= f_deg;
        PORTD |= g_deg; 
        _delay_ms(del);         
    }

    if (charbyte=='5'||charbyte==5){
        PORTD |= a_deg;
        PORTD |= c_deg;
        PORTD |= d_deg;
        PORTD |= f_deg; 
        PORTD |= g_deg; 
        _delay_ms(del);                 
    }

    if (charbyte=='6'||charbyte==6){
        PORTD |= a_deg;
        PORTD |= c_deg;
        PORTD |= d_deg;        
        PORTD |= e_deg;
        PORTD |= f_deg; 
        PORTD |= g_deg; 
        _delay_ms(del);                
    }

    if (charbyte=='7'||charbyte==7){
        PORTD |= a_deg;
        PORTD |= b_deg;
        PORTD |= c_deg;
        _delay_ms(del);         
    }

    if (charbyte=='8'||charbyte==8){
        PORTD |= a_deg;
        PORTD |= b_deg;
        PORTD |= c_deg;
        PORTD |= d_deg;        
        PORTD |= e_deg;
        PORTD |= f_deg; 
        PORTD |= g_deg; 
        _delay_ms(del);          
    }

    if (charbyte=='9'||charbyte==9){
        PORTD |= a_deg;
        PORTD |= b_deg;
        PORTD |= c_deg;
        PORTD |= f_deg; 
        PORTD |= g_deg; 
        _delay_ms(del);          
    }

                
    if (charbyte=='a'){
        PORTD |= a_deg;
        PORTD |= b_deg;
        PORTD |= c_deg;
        PORTD |= e_deg;
        PORTD |= f_deg;
        PORTD |= g_deg; 
        _delay_ms(del);                
    }

    if (charbyte=='b'){
        PORTD |= c_deg;
        PORTD |= d_deg;
        PORTD |= e_deg;
        PORTD |= f_deg;
        PORTD |= g_deg;
        _delay_ms(del);         
    }

    if (charbyte=='c'){
        PORTD |= a_deg;
        PORTD |= d_deg;
        PORTD |= e_deg;
        PORTD |= f_deg;
        _delay_ms(del);         
    }

    if (charbyte=='d'){
        PORTD |= b_deg;
        PORTD |= c_deg;
        PORTD |= d_deg;
        PORTD |= e_deg;
        PORTD |= g_deg; 
        _delay_ms(del);                
    }

    if (charbyte=='e'){
        PORTD |= a_deg;
        PORTD |= d_deg;
        PORTD |= e_deg;
        PORTD |= f_deg;
        PORTD |= g_deg; 
        _delay_ms(del);                
    }

    if (charbyte=='f'){
        PORTD |= a_deg;
        PORTD |= e_deg;
        PORTD |= f_deg;
        PORTD |= g_deg; 
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
//auto increment which segment is on, wrap if at end
void incr_seg(void){
    if (dgt_idx<7){
        dgt_idx++; 
    }
    else{    
        dgt_idx=0;
    }
    PORTB = (1 << dgt_idx);
}

/********************/
//auto decrement which segment is on, wrap if at end
void decr_seg(void){
    if (dgt_idx>0){
        dgt_idx--; 
    }
    else{    
        dgt_idx=7;
    }
    PORTB = (1 << dgt_idx);
}

/********************/
//auto increment which digit is on, wrap if at end
void incr_digit(void){
    if (seg_idx<3){
        seg_idx++; 
    }
    else{    
        seg_idx=0;
    }
    PORTB = (1 << seg_idx);
}

/********************/
//auto decrement which digit is on, wrap if at end
void decr_digit(void){
    if (seg_idx>1){
        seg_idx--; 
    }
    else{    
        seg_idx=3;
    }
    PORTB = (1 << seg_idx);
}

/********************/
//test function - flash all digits left to right
void cyclon(void)
{
    int a = 0;
    
    PORTD = 0xff; //turn on all segments 

    for(a=0;a<4;a++){
        incr_digit();
        _delay_ms(lngdel);    
    }
    for(a=0;a<4;a++){
        decr_digit();
        _delay_ms(lngdel);        
    }

}


/********************/
//test function -
void test_chars(void){
        map_character('0');
       _delay_ms(lngdel);
       PORTD = 0x00;

       map_character('1');
       _delay_ms(lngdel);
       PORTD = 0x00;

       map_character('2');
       _delay_ms(lngdel);
       PORTD = 0x00;

       map_character('3');
       _delay_ms(lngdel);
       PORTD = 0x00;

       map_character('4');
       _delay_ms(lngdel);
       PORTD = 0x00;

       map_character('5');
       _delay_ms(lngdel);
       PORTD = 0x00;

       map_character('6');
       _delay_ms(lngdel);
       PORTD = 0x00;

       map_character('7');
       _delay_ms(lngdel);
       PORTD = 0x00;

       map_character('8');
       _delay_ms(lngdel);
       PORTD = 0x00;

       map_character('9');
       _delay_ms(lngdel);
       PORTD = 0x00;
                     
       map_character('a');
       _delay_ms(lngdel);
       PORTD = 0x00;

       map_character('b');
       _delay_ms(lngdel);
       PORTD = 0x00;

       map_character('c');
       _delay_ms(lngdel);
       PORTD = 0x00;

       map_character('d');
       _delay_ms(lngdel);
       PORTD = 0x00;

       map_character('e');
       _delay_ms(lngdel);
       PORTD = 0x00;

       map_character('f');
       _delay_ms(lngdel);
       PORTD = 0x00;

       incr_digit();

}

/********************/
//test to see all bits of display flash
void cycle_all(void)
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

   /*
   if (charbyte<=10){
       ones_digit = charbyte;
   }
   if (charbyte>10&&charbyte<=100){
         tens_digit = charbyte/10; 
         ones_digit = charbyte%10;
   }
   if (charbyte>100&&charbyte<=1000){
         hndr_digit = charbyte/100;
         tens_digit = charbyte%(charbyte/100);
         ones_digit = charbyte%10;   
   }
   if (charbyte>1000&&charbyte<=9999){
         thos_digit = charbyte/1000;    
         hndr_digit = charbyte/100;
         tens_digit = charbyte/10;
         ones_digit = charbyte%10;   
   }
   */

    unsigned short val;
    unsigned char i;

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
   map_character(ones_digit);
   PORTD = 0x00; 
   incr_digit();
   
   //----------  
   map_character(tens_digit);
   PORTD = 0x00; 
   incr_digit();
   //----------  
   map_character(hndr_digit);
   PORTD = 0x00; 
   incr_digit();
 
   //----------  
   map_character(thos_digit);
   PORTD = 0x00; 
   incr_digit();         
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

   uint16_t a = 0; //count actuall inputs 
   uint16_t by3 = 0; //count by 120 degrees (thirds of a circle)

   
   uint8_t state = 0;
   uint8_t last_st = 0;

   while (1)
   {
      //for(a=0;a<9999;a++){
      if (PINB & 0x10 ){
          state = 1;
          
          if (state!=last_st)
          {
              a++;//iterate count if input high 
              if (a>=3){
                a=0;by3++;
              }           
          }
          
          //cheapo switch debouncing - redraw display and delay
          //draw the display 10 times while we are waiting for switch debouce to avoid dimming
          int x = 0;for (x=0;x<10;x++){map_numeric(by3);}

      }else{
        state = 0;
      }
      last_st = state;


      map_numeric(by3);
      //}

   }

} 

/////////////////



