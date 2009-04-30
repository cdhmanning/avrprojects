#ifndef __INT_UART_H__
#define __INT_UART_H__

#include <stdint.h>

void intuart_Init(uint8_t brg);

uint8_t intuart_Receive(uint8_t *b);

void intuart_TxQueuePut(uint8_t b);
void intuart_TxQueuePutHex(uint8_t b);
void intuart_TxQueuePutStr(const uint8_t *str);

#endif



