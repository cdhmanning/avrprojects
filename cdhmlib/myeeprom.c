#include "myeeprom.h"

#include "myinterrupts.h"

#include <avr/io.h>

void eeprom_Write(uint16_t addr, uint8_t *data, uint8_t nBytes)
{
  uint8_t enabled;
  
  while(nBytes > 0){
    while(EECR & (1<<EEPE)){
      /* spin */
    }
    EEARH = (addr >> 8) & 0xff;
    EEARL = addr & 0xff;
    EEDR = *data;
    enabled = interrupt_GetAndDisable();
    EECR |= (1<<EEMPE);
    EECR |= (1<<EEPE);
    interrupt_Enable(enabled);
    
    addr++;
    data++;
    nBytes--;
  }
}

void eeprom_Read(uint16_t addr, uint8_t *data, uint8_t nBytes)
{
  while(nBytes > 0){
    while(EECR & (1<<EEPE)){
      /* spin */
    }
    EEARH = (addr >> 8) & 0xff;
    EEARL = addr & 0xff;
    EECR |= (1<<EERE);
    *data = EEDR;
    
    addr++;
    data++;
    nBytes--;
  }
}
