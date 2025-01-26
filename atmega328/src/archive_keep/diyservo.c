
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>


#include <util/delay.h>
#define F_CPU 16000000L // Define software reference clock for delay duration

#define FOSC 16000000L // Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1


#define BIT_ON 0x30 //ascii 1
#define BIT_OFF 0x31 //ascii 0

#define sbi(a, b) (a) |= (1 << (b))
#define cbi(a, b) (a) &= ~(1 << (b))

  

volatile uint8_t stale;


/*
     this is a prototype to build a sensor->motor control 
     the UART is there to send sensor data back for debugging

    to read the encoder:
        uses two extenal pin interrupts for each quadrature  
        captures the state of both data lines on falling edge of either line 
        the while loop processes the captured data and does what it needs to with the incoming info 


    pinout

        PD2 - quadrature encoder A
        PD3 - quadrature encoder AMYUBRR

        PD4 - H bridge direction A (differential)   
        PD5 - H bridge direction B (differential)  
        PD6 - motor enable (PWM)

*/

uint16_t last_cnt    = 0;
uint16_t encoder_cnt = 0;

//gloabls for servo loop 
uint16_t newpos = 0;
uint8_t power   = 0;
uint8_t dir     = 0;

//uint8_t d_bounce  = 0;
uint16_t error = 0;

//what the encoder says is going on 
uint8_t encoder_dir = 0;

//what we are telling the H-bridge to do 
uint8_t motor_dir = 0;


uint8_t enc_a = 0;
uint8_t enc_b = 0;


/***********************************************/

void USART_Init( unsigned int ubrr)
{

   UBRR0H = (unsigned char)(ubrr>>8);
   UBRR0L = (unsigned char)ubrr;
   //Enable receiver and transmitter 
   UCSR0B = (1<<RXEN0)|(1<<TXEN0);
   // Set frame format: 8data, 2stop bit 
   UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}


/***********************************************/
void USART_Transmit( unsigned char data )
{
   // Wait for empty transmit buffer 
   while ( !( UCSR0A & (1<<UDRE0)) )
   ;
   //Put data into buffer, sends the data 
   UDR0 = data;
}



/***********************************************/
void USART_tx_string( char *data )
{
while ((*data != '\0'))
   {
      while (!(UCSR0A & (1 <<UDRE0)));
      UDR0 = *data;
      data++;
   }   
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
void send_txt_byte( uint8_t data, uint8_t use_newline,  uint8_t use_space){
   uint8_t i = 0;

   for (i=0; i<=7; i++) {
       if ( !!(data & (1 << (7 - i))) ){  // MSB
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
/***********************************************/

void servoinit(void)
{

    // setup pin change interrupts 
    DDRD &= ~(1 << DDD2);              // Clear the PD2 pin
    DDRD &= ~(1 << DDD3);              // Clear the PD3 pin
    PORTD |= (1 << PD3)|(1 << PD2);    // turn on the pull-ups

    // setup H bridge control 
    DDRD |= (1 << DDD4);               // set PD4 output for dir A 
    DDRD |= (1 << DDD5);               // set PD5 output for dir B 
    DDRD |= (1 << DDD6);               // set PD6 output for ENABLE / PWM 

    // disable all motor driver control lines 
    cbi(PORTD, 4);  //bridge A    
    cbi(PORTD, 5);  //bridge B
    cbi(PORTD, 6);  //enable 

    //TODO - need input!!
    //                // input direction  
    //                // input step   ?
    //                // input enable ?


    // setup interrupts for reading two quadrature lines  
    // Interrupt 0 Sense Control
    EICRA |= (1 << ISC01); // trigger on falling edge
    // Interrupt 1 Sense Control
    EICRA |= (1 << ISC11); // trigger on falling edge
    // External Interrupt Mask Register
    EIMSK |= (1 << INT0)|(1 << INT1);   // Turns on INT0 and INT1

    // setup PWM  
    OCR0A = 0;                             // set PWM for 50% duty cycle
    TCCR0A |= (1 << COM0A1);               // set none-inverting mode
    TCCR0A |= (1 << WGM01) | (1 << WGM00); // set fast PWM Mode
    TCCR0B |= (1 << CS01);                 // set prescaler to 8 and starts PWM
    
    // only look at the freshest of data 
    //the interrupt will tell us when its ready 
    stale=1;
}



/***********************************************/
/***********************************************/

/*
    ARGS:
        power is 0-255
        dir is 0 or 1 
        pulses is number of encoder counts  
*/
void update_motor()
{
    //uint16_t wiggle = 2;

    if(encoder_cnt<newpos){
        dir=1;
    }else{
        dir=0;
    }

    //set motor control direction 
    //caluclate the new position of motor based on encoder  
    if(dir==0){
        cbi(PORTD, 4);  //bridge A    
        sbi(PORTD, 5);  //bridge B
    }else{
        sbi(PORTD, 4);  //bridge A    
        cbi(PORTD, 5);  //bridge B
    }


    //if((encoder_cnt+wiggle)>newpos&&(encoder_cnt-wiggle)<newpos){
    if(encoder_cnt!=newpos){
        OCR0A = power;
    }else{
        OCR0A = 0;
        cbi(PORTD, 4);  //bridge A    
        cbi(PORTD, 5);  //bridge B        
    }



}


/***********************************************/
/***********************************************/

//requires two pin change interrupts to properly work 
//these are to read quadrature encoder data 
void servoloop(void)
{


    while(1)
    {  
        if(stale==0)
        {
          if (enc_a) {
              encoder_dir=0;
              encoder_cnt++;
          }else if(enc_b)
          {
              encoder_dir=1;
              //hack for unsigned int - debug 
              if(encoder_cnt>0){
                  encoder_cnt--;
              }
          }
          stale=1;       
        }    

        //we may assume encoder data is "good" here - do what thou wilt 
        if(stale==1)
        {
            //if(encoder_cnt!=last_cnt)
            //{  
                //slope = (output_end - output_start) / (input_end - input_start);
                //output = output_start + slope * (input - input_start);

                error = abs(encoder_cnt-newpos);
                //power = abs(error/4);

                //4 DEBUG'IN
                if(0){ 
                    //USART_Transmit(encoder_dir);  
                    
                    send_txt_2bytes( encoder_cnt, true, true);    
                    send_txt_2bytes( error, true, true); 
                    
                    send_txt_2bytes( power, true, true); 
                    USART_Transmit( 0xa ); //CHAR_TERM = new line  
                    USART_Transmit( 0xd ); //0xd = carriage return
                }else{
                    update_motor(); 
                }

            //    last_cnt = encoder_cnt; 
            //}   
        } 

    }
}


 
int main(){
    
    //setup uart
    USART_Init(MYUBRR);

    power   = 170;
    dir     = 1;
    
    encoder_cnt = 500;
    error       = 0;


    newpos = 500;
    
    stale=0;

    sei(); // turn on interrupts
    servoinit();
    servoloop();


}
 

/***********************************************/
/***********************************************/

// read the state of the two quadrature lines on falling edge 
// capture the data so the main loop can process
 

ISR (INT0_vect)
{
    enc_a = (PIND & (1 << PIND2)) == (1 << PIND2);
    enc_b = (PIND & (1 << PIND3)) == (1 << PIND3);
    stale=0;
}
 
ISR (INT1_vect)
{
    enc_a = (PIND & (1 << PIND2)) == (1 << PIND2);
    enc_b = (PIND & (1 << PIND3)) == (1 << PIND3);
    stale=0;
}
 



/***********************************************/
/*

uint8_t read_gray_code_from_encoder(void ) 
{ 
 uint8_t val=0; 

  if(!bit_is_clear(PIND, PD2)) 
    val |= (1<<1); 

  if(!bit_is_clear(PIND, PD3)) 
    val |= (1<<0); 

  return val; 
}

*/
