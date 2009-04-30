#include "avr/io.h"
#include "avr/interrupt.h"
#include "timer0.h"


void Timer0Initialise(void)
{
	
	OCR0A =153;
	TCCR0A = (1<<WGM01) | (0<<WGM00);
	TCCR0B=3;
	TIMSK0=(1<<OCIE0A);
}

static volatile unsigned long milliseconds;

ISR(TIMER0_COMPA_vect)
{
	milliseconds++;
}

unsigned long GetMilliseconds(void)
{
	unsigned long retval;
	
	cli();
	retval = milliseconds;
	sei();
	
	return retval;
	
}

void WaitMilliseconds(unsigned long n)
{
	unsigned long endtime = n + GetMilliseconds();
	while(GetMilliseconds() < endtime){
	}
}

