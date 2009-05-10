
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "myinterrupts.h"
#include "timer0.h"
#include "myusart.h"



uint8_t ReadADC(uint8_t channel)
{
//	static uint8_t x;
//	return x++;
	
	while((ADCSRA & 0x40)) {/* spin */ }

	// Start next conversion
	ADMUX = 0x20 | (channel & 7);	// External AVcc Ref, left justify, set mux
//	ADCSRA = 0x87;	// Enable, set slowest clock
//	ADCSRA = 0xC7;	// Enable, start conversion set slowest clock
	ADCSRA = 0x85;	// Enable, set /32 clock
	ADCSRA = 0xC5;	// Enable, start conversion set /32 clock

	while((ADCSRA & 0x40)) {/* spin */ }

	ADCSRA = 0x85;	// Enable, set /32 clock
	ADCSRA = 0xC5;	// Enable, start conversion set /32 clock

	while((ADCSRA & 0x40)) {/* spin */ }
	
	return ADCH;
}


void ShowIntVal(uint8_t val)
{
	uint8_t x;
	x = val/100;
	usart_TxQueuePut('0'+x);
	val = val % 100;
	x = val/10;
	usart_TxQueuePut('0'+x);
	val = val % 10;
	x = val;
	usart_TxQueuePut('0'+x);
	usart_TxQueuePut(' ');
}

#define N_SAMPLES 10

static uint8_t sample[N_SAMPLES];

static void RED(void)
{
	DDRB = 1 | (1<<3);
	PORTB = (1<<3);
}
static void GREEN(void)
{
	DDRB = 1 | (1<<4);
	PORTB = (1<<4);
}
static void BLUE(void)
{
	DDRB = 1 | (1<<5);
	PORTB = (1<<5);
}

static void OFF(void)
{
	DDRB = 0;
	PORTB = 0;
}

int
main (void)
{
	uint8_t on_off;
	uint8_t val;
	uint16_t i;
	
	// Set clock to full speed
	CLKPR = 0x80;
	CLKPR = 0x00;

	// Init timer
	Timer0Initialise ();
	
	usart_Init(12);
	usart_TxQueuePutStr("\n\r"
			      "LDR Test 5ms\n\r"
			      "-----------------\n\r");

	// Initialise port pins
	DDRB = 0x01;
	DDRC = 0;
	DDRD = 0;


	interrupt_Enable(1);

	while (1) {
		RED();
		//WaitMilliseconds(1);
		val = ReadADC(1);
		OFF();
		ShowIntVal(val);
 
		GREEN();
		val = ReadADC(1);
		OFF();
		ShowIntVal(val);

		BLUE();
		val = ReadADC(1);
		OFF();
		ShowIntVal(val);

		usart_TxQueuePut('\n');
		usart_TxQueuePut('\r');
		WaitMilliseconds(500);		


	}
}

