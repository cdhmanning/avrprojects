#ifndef __SERVO_H__
#define __SERVO_H__

#include <stdint.h>

void servo_Set(uint8_t channel, uint8_t val);
void servo_Free(uint8_t channel);
void servo_Initialise(void);


#endif
