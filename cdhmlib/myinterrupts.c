#include "myinterrupts.h"
#include <avr/io.h>
uint8_t interrupt_GetAndDisable(void)
{
  uint8_t tmp = SREG & 0x80;
  SREG &= ~ 0x80;
  return tmp;
}

void interrupt_Enable(uint8_t enable)
{
  if(enable)
    SREG |= 0x80;
}