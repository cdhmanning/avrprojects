#ifndef __MYEEPROM_H__
#define __MYEEPROM_H__

#include <stdint.h>

void eeprom_Write(uint16_t addr, uint8_t *data, uint8_t nBytes);
void eeprom_Read (uint16_t addr, uint8_t *data, uint8_t nBytes);

#endif

