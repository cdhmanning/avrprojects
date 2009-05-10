#ifndef __INT_UART_H__
#define __INT_UART_H__

#include <stdint.h>

void usart_Init(uint8_t brg);

uint8_t usart_Receive(uint8_t *b);

void usart_TxQueuePut(uint8_t b);
void usart_TxQueuePutHex(uint8_t b);
void usart_TxQueuePutDec(uint8_t b);
void usart_TxQueuePutStr(const uint8_t *str);

#endif



