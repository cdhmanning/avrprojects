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

void intuart_Init(uint8_t brg)
{
  UBRR0L=brg;
  UBRR0H=0;
  UCSR0B = (_BV(TXCIE0) | _BV(RXEN0) | _BV(TXEN0));
  
  txqholding = 0;
  txqtail = 0;
  txqhead = 0;
}

uint8_t intuart_Receive(uint8_t *b)
{
  if(!(UCSR0A & _BV(RXC0)))
    return 0;
  *b = UDR0;
  return 1;
}


static uint8_t intuart_TxQueueGet(void)
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
    UDR0 = intuart_TxQueueGet();
  else
    txSending = 0;
}

void intuart_TxQueuePut(uint8_t b)
{
  uint8_t intEnabled = interrupt_GetAndDisable();
  
  if(txqholding == TX_QUEUE_SIZE)
    intuart_TxQueueGet();
  txq[txqhead] = b;
  txqholding++;
  txqhead++;
  if(txqhead >= TX_QUEUE_SIZE)
    txqhead = 0;
    
  if(!txSending){
    txSending = 1;
    UDR0 = intuart_TxQueueGet();
  }
    
  interrupt_Enable(intEnabled);
}

const char hexStr[]= "0123456789ABCDEF";

void intuart_TxQueuePutHex(uint8_t x)
{
  intuart_TxQueuePut('[');
  intuart_TxQueuePut(hexStr[(x >> 4) & 0xf]);
  intuart_TxQueuePut(hexStr[x & 0xf]);
  intuart_TxQueuePut(']');
}

void intuart_TxQueuePutStr(const uint8_t *str)
{
  while(*str){
    intuart_TxQueuePut(*str);
    str++;
  }
}
