
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "myinterrupts.h"
#include "timer0.h"
#include "myusart.h"
#include "myeeprom.h"


int
main (void)
{
	uint8_t ret, b, x;
	uint8_t ee[2];
	
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
			ee[0] = b;
			ee[1] = ~b;
			eeprom_Write(66,ee,2);
			ee[0] = ee[1] = 0;
			
		}
		eeprom_Read(66,ee,2);
		usart_TxQueuePutDec(b);
		usart_TxQueuePutHex(b);
		usart_TxQueuePut('!');
		usart_TxQueuePutHex(ee[0]);
		usart_TxQueuePutHex(ee[1]);
		usart_TxQueuePutStr("\n\r");
		WaitMilliseconds(500);		


	}
}

