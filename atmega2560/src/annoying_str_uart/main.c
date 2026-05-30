#include <avr/io.h>
#include <avr/pgmspace.h>

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define F_CPU 16000000UL  //AVR Clock Speed in MHZ
#define FOSC  16000000UL  // Clock Speed

#include <util/delay.h>


//#define MYUBRR 16000000/16/(115200-1) //THIS WORKS FOR 115200

#define MYUBRR 16000000/16/(57600-1)    //THIS WORKS FOR 57600

/***********************************************/
void USART_Init( unsigned int ubrr)
{

    
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;
 
    //Enable receiver and transmitter 
    UCSR0B = (1<<RXEN0)|(1<<TXEN0);

    //enablbe 2X speed
    UCSR0A |= (1<<U2X0);

}


/***********************************************/
void UART_write_str_pgm(const char* s)
{
    uint8_t c;

    for (uint8_t i=0; i < strlen_P(s); i++)
    {
        c = pgm_read_byte(&(s[i]));
        while (( UCSR0A & (1<<UDRE0))  == 0){};
        UDR0 = c;          
    }

}  



/***********************************************/
//this works for a single character 

void UART_transmit( unsigned char data )
{
    while ( !( UCSR0A & (1<<UDRE0)) );
    UDR0 = data;
}


/***********************************************/
//  DOES NOT WORK IF STRING IS LONGER THAN 2 or 3 CHARS 
 
void UART_write_str1(char *data) 
{ 

    while(*data)
    { 
        while ( !( UCSR0A & (1<<UDRE0)) );
        UDR0 = *data++; 
    }

} 
  

/***********************************************/
 
//  DOES NOT WORK IF STRING IS LONGER THAN 2 or 3 CHARS 
void UART_write_str2(char *data) 
{ 
    int i =0;
    while (data[i] != 0x00)
    {
        //UART_transmit(data[i]);
        while ( !( UCSR0A & (1<<UDRE0)) );
        UDR0 = *data++; 

        i++;
    }

}
 
/***********************************************/
void UART_write_str3(char *data) 
{ 
    int i =0;
    while (data[i] != 0x00)
    {
        UART_transmit(data[i]);
        i++;
    }

}

/***********************************************/
void UART_write_str4(char *s)
{
    char ch;

    while ((ch = *s++) != '\0')
        UART_transmit(ch);
}

 

/***********************************************/
//setup some data to go into flash to send to uart 

//CHAR ARRAYS MUST BE GLOBAL TO WORK WITH PROGMEM!!!

const char string_1[] PROGMEM = "this works fine";
const char string_2[] PROGMEM = "so does this";

PGM_P const string_table[] PROGMEM = 
{
    string_1,
    string_2
};


/***********************************************/

int main (void)
{
 

    USART_Init(MYUBRR);


    //I tried fixed size of array ([10], etc) make no difference 
    //I tried compiling with g++ vs gcc , compiler flags etc 
    //The clue is that compiler flag  "-0s" DID have some affect on the problem (doubled buffer size from 2 to 4) 
    //nothing "fixes" it 

    //I tried global (outside function), static, const, volatile, inline : Makes no difference 
    //however PROGMEM variables do need to be global (outside a function)


    char buffer1[20];

    while (1)
    {

        // this works fine for a single character 
        //UART_transmit(0x41);

        
        // THIS WORKS FINE FOR LONGER STRINGS (FROM FLASH)
        //strcpy_P(buffer1, (PGM_P)pgm_read_word(&(string_table[0])));
        //UART_write_str1(buffer1);


        // THESE ONLY WORK IF ARRAY IS 4 CHARS OR LESS
        //NOTHING SEEMS TO FIX IT 
        //I NOTICED THAT THE COMPILER OPTIMIZE FLAG -Os affects it 


        //strcpy(buffer1, "sus");
        //UART_write_str3(buffer1);

        //works with <4 chars 
        //char buffer2[] = "hi";
        //UART_write_str1(buffer2);

        //works with <4 chars 
        //char buffer2[] = "h";
        //UART_write_str2(buffer2);

        //works with <4 chars 
        //char buffer2[] = "heieee";
        //UART_write_str3(buffer2);
 
        //works with <4 chars 
        strcpy(buffer1, "kei");
        UART_write_str1(buffer1);


        _delay_ms(800); 
    }
    
} 
