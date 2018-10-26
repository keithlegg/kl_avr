#ifndef _ST7735uart_
#define _ST7735uart_


/***********************************************/

void USART_Init(void);

void flood_pattern(void);

/***********************************************/

uint8_t USART_receive(void);

/***********************************************/

void USART_Transmit( char data );

/***********************************************/
void send_txt_byte( uint8_t data, uint8_t use_newline);

/***********************************************/
void print_byte( uint8_t data);

/***********************************************/
void print_byte_16( uint16_t data);

/***********************************************/
//debug experimental - cant seem to make this work!
void USART_tx_string( char data[]);

/***********************************************/
void USARTWriteStr(char *data); 

/***********************************************/

unsigned char uartrecieve(unsigned char *x, unsigned char size);
//*******************************************//
void echo_uart();


#endif