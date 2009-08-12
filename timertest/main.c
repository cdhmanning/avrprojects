
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
	
	// Init timer
	Timer0Initialise ();
	
	usart_Init(12);
	interrupt_Enable(1);
	b='0';
 
	while (1) {
		
		usart_TxQueuePut(b);
		usart_TxQueuePut('\n');
		usart_TxQueuePut('\r');
		WaitMilliseconds(1000);
		b++;
		if(b > '9')
		        b = '0';


	}
}

