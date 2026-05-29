



//CHAR ARRAYS MUST BE GLOBAL TO WORK WITH PROGMEM!!!
//const char foo[] PROGMEM = "keith finally figured out how to make long f-ing strings... grr"; 
//const char serial_buffer[100] PROGMEM; 




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


