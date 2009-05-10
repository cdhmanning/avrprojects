
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "myinterrupts.h"
#include "timer0.h"
#include "myusart.h"


int
main (void)
{
	uint8_t ret, b, x;
	
	// Set clock to full speed
	CLKPR = 0x80;
	CLKPR = 0x00;

	// Init timer
	Timer0Initialise ();
	
	usart_Init(12);
	usart_TxQueuePutStr("\n\r"
			      "UART test\n\r"
			      "-----------------\n\r");


	interrupt_Enable(1);
	b = 5;
 
	while (1) {
		ret = usart_Receive(&x);
		if(ret){
			usart_TxQueuePut('*');
			b = x;
		}
		usart_TxQueuePutDec(b);
		usart_TxQueuePutHex(b);
		usart_TxQueuePutStr("\n\r");
		WaitMilliseconds(500);		


	}
}

