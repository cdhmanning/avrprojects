#ifndef __MYINTERRUPTS_H__
#define __MYINTERRUPTS_H__

#include <stdint.h>
uint8_t interrupt_GetAndDisable(void);
void interrupt_Enable(uint8_t enable);


#endif
