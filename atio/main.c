
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "myinterrupts.h"
#include "twi_avr.h"
#include "timer0.h"
#include "myusart.h"

#define SENSOR_SLAVE_ADDRESS 0x01
const char SensorVersion[8] = "1.1";
const char SensorName[8] = "MBOYS";
const char SensorType[8] = "ATIO";


#define N_ADC	4
unsigned char adcVal[N_ADC];
unsigned char curChan;

void
ADCProcess (void)
{
	if (!(ADCSRA & 0x40)) {

		// Conversion not in progress
		if (curChan < N_ADC)
			adcVal[curChan] = ADCH;
		curChan++;
		if (curChan >= N_ADC)
			curChan = 0;

		// Start next conversion
		ADMUX = 0x20 | curChan;	// External AVcc Ref, left justify, set mux
		ADCSRA = 0x87;	// Enable, set slowest clock
		ADCSRA = 0xC7;	// Enable, start conversion set slowest clock
	}
}

void
Spin (void)
{
	ADCProcess ();
} 

static unsigned char messageBuf[6];

void
ProcessMessage (void)
{

	/* Have received a message */
	
	usart_TxQueuePutStr("\n\rMsg:");

	usart_TxQueuePutHex(messageBuf[0]);
	usart_TxQueuePutHex(messageBuf[1]);
	
	/* Command received */
	if (messageBuf[0] == 0x40)
		DDRB = messageBuf[1];
	if (messageBuf[0] == 0x41)
		PORTB = messageBuf[1];

	/*
	 * Info request received
	 */
	if (messageBuf[0] == 0x00) {
		twi_sl_update(SensorVersion,sizeof(SensorVersion));
	} else if (messageBuf[0] == 0x08) {
		twi_sl_update(SensorName, sizeof (SensorName));
	} else if (messageBuf[0] == 0x10) {
		twi_sl_update(SensorType, sizeof (SensorType));
	} else if (messageBuf[0] == 0x18) {
		messageBuf[0] = adcVal[0];
		messageBuf[1] = adcVal[1];
		messageBuf[2] = adcVal[2];
		messageBuf[3] = adcVal[3];
		messageBuf[4] = PINB;
		twi_sl_update(messageBuf, 5);
	}

	usart_TxQueuePutStr("\n\r");
}

static unsigned char slaveAddress;

int
main (void)
{
	uint16_t counter;
		 
	// Set clock to full speed
	CLKPR = 0x80;
	CLKPR = 0x00;

	// Init timer
	Timer0Initialise ();
	
	usart_Init(12);
	usart_TxQueuePutStr("\n\r"
			      "MBOYS ATIO sensor\n\r"
			      "-----------------\n\r");
	
	

	// Initialise port pins
	DDRB = 0;
	DDRC = 0;
	DDRD = 0;
	PINB = 0;
	PINC = 0;
	PIND = 0xFF;

	// Own TWI slave address
	slaveAddress = SENSOR_SLAVE_ADDRESS;

	// Initialise TWI module for slave operation. Include address and/or enable General Call.
	twi_init(slaveAddress<<1);

	interrupt_Enable(1);

	// Start the TWI transceiver to enable reseption of the first command from the TWI Master.

	// This loop runs forever. If the TWI is busy the execution will just continue doing other operations.
	while (1) {

		if (twi_sl_poll(messageBuf,2)) {
			// Ends up here if the last operation was a reception as Slave Address Match			
			ProcessMessage ();
		}
		// Do something else while waiting for the TWI transceiver to complete.    
		Spin ();
		counter++;
		if(counter> 50000){
			counter = 0;
			usart_TxQueuePut('.');
		}
	}
}

