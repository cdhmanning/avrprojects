#include "myusart.h"
#include "myinterrupts.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define TX_QUEUE_SIZE 128

static volatile uint8_t txqholding;
static volatile uint8_t txqhead;
static volatile uint8_t txqtail;
static volatile uint8_t txq[TX_QUEUE_SIZE];
static volatile uint8_t txSending;

void usart_Init(uint8_t brg)
{
  UBRR0L=brg;
  UBRR0H=0;
  UCSR0B = (_BV(TXCIE0) | _BV(RXEN0) | _BV(TXEN0));
  
  txqholding = 0;
  txqtail = 0;
  txqhead = 0;
}

uint8_t usart_Receive(uint8_t *b)
{
  if(!(UCSR0A & _BV(RXC0)))
    return 0;
  *b = UDR0;
  return 1;
}


static uint8_t usart_TxQueueGet(void)
{
  uint8_t b = 0;
  
  if(txqholding){
    b = txq[txqtail];
    txqtail++;
    if(txqtail >= TX_QUEUE_SIZE)
      txqtail = 0;
    txqholding--;
  }
  return b;
}

ISR(USART_TX_vect)
{
  /* This is the TX Complete interupt */
  if(txqholding)
    UDR0 = usart_TxQueueGet();
  else
    txSending = 0;
}

void usart_TxQueuePut(uint8_t b)
{
  uint8_t intEnabled = interrupt_GetAndDisable();
  
  if(txqholding == TX_QUEUE_SIZE)
    usart_TxQueueGet();
  txq[txqhead] = b;
  txqholding++;
  txqhead++;
  if(txqhead >= TX_QUEUE_SIZE)
    txqhead = 0;
    
  if(!txSending){
    txSending = 1;
    UDR0 = usart_TxQueueGet();
  }
    
  interrupt_Enable(intEnabled);
}

const char hexStr[]= "0123456789ABCDEF";

void usart_TxQueuePutHex(uint8_t x)
{
  usart_TxQueuePut('[');
  usart_TxQueuePut(hexStr[(x >> 4) & 0x0f]);
  usart_TxQueuePut(hexStr[x & 0x0f]);
  usart_TxQueuePut(']');
}
void usart_TxQueuePutDec(uint8_t x)
{
  usart_TxQueuePut('0' + (x /100));
  x %= 100;
  usart_TxQueuePut('0' + (x /10));
  x %= 10;
  usart_TxQueuePut('0' + x);
  usart_TxQueuePut(' ');
}

void usart_TxQueuePutStr(const uint8_t *str)
{
  while(*str){
    usart_TxQueuePut(*str);
    str++;
  }
}
