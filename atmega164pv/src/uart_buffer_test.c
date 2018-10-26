#include "../include/kl_gps.h"

/*

  GPS DEMO CODE - READ AND PROCESS SOME DATA 
  Interrupt driven UART ring buffer for atmega164p
  Keith Legg 8,8,2013


  // SAMPLE DATA LOOKS LIKE THIS :
  // $GPRMC,054123.000,A,4402.4926,N,12257.6825,W,0.00,152.38,080813,,,A*73

*/


volatile uint8_t bufferReady   = FALSE;
volatile uint8_t bufIndex      =     0;
volatile uint8_t uartRXBuf[RX_BUF_LEN];

//volatile uint8_t linecount = 0; //grab a few lines and process with python?

void wait_begin(){
   uint8_t x =0;

   if (bufIndex==0){
       while(x!='\n'){
	  x = USART_receive_1(); 
       }
       bufferReady = FALSE;
       UCSR1B |= (1 << RXCIE1);  

    }

}

int main()
{
    //activate UART hardware/enable interrupts
    USART_init_0(UBBRVAL);    
    USART_init_1(UBBRVAL2);
 
    //turn on global interrupts
    sei(); 
    
    uint8_t y =0;
  
   ////////////////////
   //activate interrupts when a new line begins

    wait_begin();

    while (TRUE)
    {

       // y = USART_receive_0();
       //USART_transmit_0(y);
       //if (y==0x61){

        //_delay_ms(4333);


       /////////////////////////////////
        if(bufferReady) {
            parse_gps_data(&uartRXBuf[0], bufIndex);
            //send_string_0_2(uartRXBuf,100);

            /////////////////
              
            //reset the buffer state 
            bufIndex    = 0;
            bufferReady = FALSE;

            //re-enable the interrupt.
            //while(x!='\n'){
	    //	  x = USART_receive_1(); 
	    //}
	    //UCSR1B |= (1 << RXCIE1); 
            wait_begin();
        }

    }

    return 1;
}
///////////////


void parse_gps_data(volatile uint8_t *buffer, volatile uint8_t len)
{
   switch(*(buffer)) {

        case '$':
        {
            
            //send_string_0_2(uartRXBuf,15);
     
            //MY FIRST USE OF STRSTR THAT WORKS!
            //char test[] = "$GPRMC";
            //char test[] = "$GPGGA";
            //char bar = &test;
            //char * foo = strstr(bar,buffer);  

 
            //same but no passing of pointer
            char* pass1 = strstr("$",buffer);  
            if(pass1){
                 send_string_0_2(buffer,65);
                 //char* pass2 = strstr(",N,",buffer);  
                 //if (pass2){
                 //   send_string_0(buffer);
                // }
            }


            //send_string_0(buffer);
            ////////////////////

            //USART_transmit_0( buffer[0] ); //fer debuggin
            //send_string_0(buffer);

            //bufIndex    = 0;
            //bufferReady = FALSE;
            
            break;
        }
   }



   //ERROR:
   //   nack();
   //   return;

}

///////////////
void send_string_0(char s[])
{
   int i =0;
   
   while (s[i] != 0x00)
   {
      USART_transmit_0(s[i]);
      i++;
   }
} 

void send_string_0_2(char s[],int num)
{
   int i =0;
   
   for (i=0;i<num;i++)
   {
      if (s[i]!=0x00){
         USART_transmit_0(s[i]);
      }
   }
   USART_transmit_0('\n');

} 



ISR(USART1_RX_vect, ISR_BLOCK)
{
    uint8_t c = UDR1;    
 
    //when we hit the end of the line
    if(c == '\n') { 
	   bufferReady = TRUE;
           // Disable interrupts untill process the buffer.
           UCSR1B&=~(1 << RXCIE1); //DMV way 

    }else if(bufIndex == (RX_BUF_LEN - 1)) {
		bufIndex = 0;
		nackfull(); //buffer is full error
		return;
    }


    uartRXBuf[bufIndex++] = c;
    //USART_transmit_0(c);  //fer debuggin
 
}


/////////////////////////////////////

//BACKUP - NICE BUT NOT WORKING 
/*
ISR(USART1_RX_vect, ISR_BLOCK)
{
    //look at each char received
    uint8_t c = USART_receive_1();

    //...until we hit the end of the line
    if(c == '\n') {
        //if buffer has data 
        if(bufIndex) {
            // Disable UART RX interrupts while we process the buffer.
            //UCSR1B &=(0 << RXCIE1); 

            bufferReady = TRUE;
            return;
        } else {
            nack();
            return;
        }
    } else if(bufIndex == (RX_BUF_LEN - 1)) {
        bufIndex = 0;
        nackfull(); //does this mean full?
        return;
    }

    //USART_transmit_0(c);     //fer debuggin
    uartRXBuf[bufIndex++] = c;
}

*/


///////////////


static void USART_init_0(uint16_t ubrr)
{
    // Load upper and lower bytes of baud rate into the UBRR register
    UBRR0H = (uint8_t)(ubrr>>8); 
    UBRR0L = (uint8_t)ubrr;      

    // Enable transmitter and receiver.
    UCSR0B |= ((1 << RXEN0)|(1 << TXEN0));

    // Enable the USART RX interrupt.
    //UCSR1B |= (1 << RXCIE1); // Enable the USART Recieve Complete interrupt (USART_RXC)

}


static void USART_transmit_0(uint8_t data)
{
    while (!(UCSR0A & (1 << UDRE0))) {/*Busy wait.*/}
    UDR0 = data;
}

static uint8_t USART_receive_0(void)
{
    while (!(UCSR0A & (1 << RXC0))) {/*Busy wait.*/}
    return UDR0;
}

///////////////

static void USART_init_1(uint16_t ubrr)
{
    UBRR1H = (uint8_t)(ubrr>>8); 
    UBRR1L = (uint8_t)ubrr;     

    // Enable transmitter and receiver.
    UCSR1B |= ((1 << RXEN1)|(1 << TXEN1));
}


static void USART_transmit_1(uint8_t data)
{
    while (!(UCSR1A & (1 << UDRE1))) {/*Busy wait.*/}
    UDR1 = data;
}


static uint8_t USART_receive_1(void)
{
    while (!(UCSR1A & (1 << RXC1))) {/*Busy wait.*/}
    return UDR1;
}



/**
* Sends the serial command acknowledge code.
*/
void ack(void)
{
    USART_transmit_0('O');
    USART_transmit_0('K');
    USART_transmit_0('\r');
    USART_transmit_0('\n');
}


/**
* Sends the serial command not acknowledge code.
*/
void nack(void)
{
    USART_transmit_0('E');
    USART_transmit_0('R');
    USART_transmit_0('R');
    USART_transmit_0('O');
    USART_transmit_0('R');
    USART_transmit_0('\r');
    USART_transmit_0('\n');
}

void nackfull(void)
{
    USART_transmit_0('B');
    USART_transmit_0('U');
    USART_transmit_0('F');
    USART_transmit_0('F');
    USART_transmit_0('R');
    USART_transmit_0('_');
    USART_transmit_0('F');
    USART_transmit_0('U');
    USART_transmit_0('L');
    USART_transmit_0('L');
    USART_transmit_0('\r');
    USART_transmit_0('\n');
}



