#include "avr/io.h"
#include "avr/interrupt.h"
#include "servo.h"

/* At first we're just using timer2 to control up to 8 servos on PortB0..7.
* Vaues passed into the servo routine are in units of 10us. ie. 100-> 1ms, 200->2ms
* If the counter is in the range 100..200 then the corresponding channel will be controlled.
* If outside that range, the channel will be skipped.
*
* Control is achieved by using counter 2
*
* Timing issues:
* We use the timer match interrupt to schedule pulses. Since we need to send out pulses
* at approx  50Hz and there are 8 channels to cycle through, we need to do a scheduling
* at approx 400Hz (2.5ms). Of that time, a pulse might take up to 2ms.
*
* THus we use two 
*
* Since the main CPU clock is 8MHz (0.125uS), the overflow must happen approx once every 20000 cycles.

*
* Driven by a 8MHz, we set up the prescaler to generate a clock with 16uS period
* Thus 1ms -> 62 counts and 2ms -> 125 counts
* The counter value is easily achieved by taking half the input value, adding a quarter.
*
*
*/


#define SERVO_SCHEDULE_COUNT 156

#define NSERVOS	8
#define SERVO_PORT	PORTB
#define SERVO_DDR	DDRB

static volatile uint8_t servo_counter[NSERVOS];
static volatile uint8_t servo_currentChannel;
static volatile uint8_t servo_currentBit;
static volatile uint8_t servo_currentMatch;

void servo_Set(uint8_t channel, uint8_t val)
{
	uint8_t channelBit;
	
	if (channel >= 8 )
		return;

	channelBit = (1 << channel);

	if(val < 100 || val > 200)
		val = 0;
	val>>=1;
	val += (val >>2);
	
	if(!(SERVO_DDR & channelBit))
		SERVO_PORT &= ~channelBit;
	SERVO_DDR |= (1 << channel);
	servo_counter[channel] = val;
}

void servo_Free(uint8_t channel)
{
	if(channel >= 8)
		return;
	servo_counter[channel] = 0;	
}

void servo_Initialise(void)
{

	// Set clock to full speed (8MHz)
	CLKPR = 0x80;
	CLKPR = 0x00;
	
	OCR2A= 100;
	TCCR2A=0x02;
	TCCR2B=0x05;
	TIMSK2=(1<<OCIE2A);
}


/*
 * The interru are used as follows.
 * The COMPA vector is used to switch off the pulse.
 */
 
ISR(TIMER2_COMPA_vect)
{
	/*
	 *We get here in one of two states:
	 * Pulse active: If so, we need to finish the pulse and set up the inter-pulse gap.
	 * Pulse inactive: Set up the next pulse if there is one.
	 */

	if(servo_currentBit){
		/* Pulse active */
		SERVO_PORT &= ~ servo_currentBit;
		servo_currentBit = 0;
		OCR2A = SERVO_SCHEDULE_COUNT - servo_currentMatch;
		TCNT2=0;
		TIFR2=2;
	} else {
		servo_currentChannel++;
		servo_currentChannel &= 7;
	
		servo_currentMatch = servo_counter[servo_currentChannel];
		if(servo_currentMatch > 0){
			OCR2A = servo_currentMatch;
			TCNT2=0;
			TIFR2=2;
			servo_currentBit = (1 << servo_currentChannel);
			SERVO_PORT |= servo_currentBit;
		} else {
			OCR2A = SERVO_SCHEDULE_COUNT;
			TCNT2=0;
			TIFR2=2;
			servo_currentBit=0;
		}
	}
		
}
