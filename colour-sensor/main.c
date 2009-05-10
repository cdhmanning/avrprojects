
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "myinterrupts.h"
#include "twi_avr.h"
#include "timer0.h"
#include "myusart.h"

#define SENSOR_SLAVE_ADDRESS 0x02
#define NCHANNELS	4
#define NCOLOURS	4 /* ambient, red, blue, green */

/* 
 * LED control on PORTB
 */
 
#define RED_PIN		(1<<5)
#define GREEN_PIN	(1<<4)
#define BLUE_PIN	(1<<3)

#define CHANNEL0_PIN	(1<<0)
#define CHANNEL1_PIN	(1<<1)
#define CHANNEL2_PIN	(1<<2)
#define CHANNEL3_PIN	(1<<6)


const char SensorVersion[8] = "1.1";
const char SensorName[8] = "MBOYS";
const char SensorType[8] = "COLOUR4";


uint8_t rawValue[NCHANNELS][NCOLOURS];

static uint8_t state;
static uint8_t channel;
static uint8_t rgb;



void SampleProcess(void)
{
	static uint8_t a,b,c;
	
	uint16_t sample_value;
	uint8_t start_sample = 0;
	
	if(ADCSRA & 0x40)
		return;

	if(channel >= NCHANNELS)
		state = 0;
	if(rgb >= NCOLOURS)
		state = 0;
	
	sample_value = ADCH;
	
	switch(state){
		case 0:
			DDRB = 0;
			break;
		case 1: /* Enable LED and do a wasted sample */
		
			
			switch(rgb){
				case 0: DDRB = 0;
					PORTB = 0;
					break;
				case 1:
					DDRB = RED_PIN;
					PORTB = RED_PIN;
					break;
				case 2:
					DDRB = GREEN_PIN;
					PORTB = GREEN_PIN;
					break;
				case 3: 
					DDRB = BLUE_PIN;
					PORTB = BLUE_PIN;
					break;
				default:
					DDRB = 0;
			}
			
			switch(channel){
				case 0:
					DDRB |= CHANNEL0_PIN;
					break;
				case 1:
					DDRB |= CHANNEL1_PIN;
					break;
				case 2:
					DDRB |= CHANNEL2_PIN;
					break;
				case 3:
					DDRB |= CHANNEL3_PIN;
					break;
			}
						
			start_sample = 1;
			state++;
			break;
		case 2: /* Start first sample */
			start_sample = 1;
			state++;
			break;
		case 3: /* Save first sample, start second sample */
			start_sample = 1;
			a= sample_value;
			state++;
			break;
		case 4: /* Save second value, start third sample */
			start_sample = 1;
			if(a < sample_value)
				b = sample_value;
			else {
				b = a;
				a = sample_value;
			}
			state++;
			break;
		case 5: /* Save third value and advance to next sample */
			if(sample_value > b)
				c = b;
			else if(sample_value < a)
				c =a;
			else
				c = sample_value;

			rawValue[channel][rgb] = c;
			
			start_sample = 1;
			rgb++;
			if(rgb > NCOLOURS){
				rgb = 0;
				channel++;
			}
			state = 1;
			break;
	}
	
	if(start_sample){
		ADMUX = 0x20 | channel;	// External AVcc Ref, left justify, set mux
		ADCSRA = 0x85;	// Enable, set clock
		ADCSRA = 0xC5;	// Enable, start conversion set clock
	}

}

void
Spin (void)
{
	static uint16_t xx;
	xx++;
	
	SampleProcess ();

	if(xx == 0 && state == 0){
		
		usart_TxQueuePutDec(rawValue[0][0]);
		usart_TxQueuePutDec(rawValue[0][1]);
		usart_TxQueuePutDec(rawValue[0][2]);
		usart_TxQueuePutDec(rawValue[0][3]);
		usart_TxQueuePutStr("\n\r");
		
		rgb = 0;
		channel = 0;
		state = 1;
	}
} 

static unsigned char messageBuf[16];

void
ProcessMessage (void)
{

	/* Have received a message */
	
	usart_TxQueuePutStr("\n\rMsg:");

	usart_TxQueuePutHex(messageBuf[0]);
	usart_TxQueuePutHex(messageBuf[1]);
	
	/* Command received */
	if (messageBuf[0] == 0x40){
	}

	/*
	 * Info request received
	 */
	if (messageBuf[0] == 0x00) {
		twi_sl_update(SensorVersion,sizeof(SensorVersion));
	} else if (messageBuf[0] == 0x08) {
		twi_sl_update(SensorName, sizeof (SensorName));
	} else if (messageBuf[0] == 0x10) {
		twi_sl_update(SensorType, sizeof (SensorType));
	} else if (messageBuf[0] == 0x20) {
		twi_sl_update(rawValue, sizeof(rawValue));
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
			      "MBOYS COLOUR sensor\n\r"
			      "-------------------\n\r");
	
	

	// Initialise port pins
	DDRB = 0;
	DDRC = 0;
	DDRD = 0;
	PINB = 0;
	PINC = 0;
	PIND = 0;

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
//			usart_TxQueuePut('.');
		}
	}
}

