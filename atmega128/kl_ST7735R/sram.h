
#ifndef _SRAMH_
#define _SRAMH_


void flood_sram(uint8_t byte);

void clear_sram(void);

/***********************************************/
uint8_t set_address(uint16_t address);

/***********************************************/
void rx_ram(uint16_t numbytes);

/***********************************************/
void tx_bytes(uint16_t startaddr, uint16_t numbytes);

/***********************************************/
uint8_t read_ram(uint16_t address);

/***********************************************/
void write_ram(uint16_t address, uint8_t byte);

/*******************************/
/*THESE USE GLOBALS INSTEAD OF RETURNING A VALUE!!!*/
void cvt_16_to_8x2(uint16_t input);

/*******************************/
uint16_t cvt_8x2_to_16(uint8_t msbin, uint8_t lsbin);

#endif