#include <avr/io.h>
#include <avr/interrupt.h>

#define RELAY_PIN (1<<1)

static void Timer0Initialise(void)
{
	
		OCR0A =153;
		TCCR0A = (1<<WGM01) | (0<<WGM00);
			TCCR0B=3;
		TIMSK0=(1<<OCIE0A);
}

volatile unsigned char msInterruptHappened;

volatile unsigned char randomCounter;

ISR(TIMER0_COMPA_vect)
{
	msInterruptHappened=1;
}

static void WaitNextMillisecond(void)
{
	do{ 
		randomCounter++;
	} while (!msInterruptHappened);
	
	msInterruptHappened=0;
}

static void WaitMilliseconds(int n)
{
	while(n>0){
		WaitNextMillisecond();
		n--;
	}
}

static void WaitSeconds(int n)
{
	while(n > 0){
		WaitMilliseconds(1000);
		n--;
	}
}




int main(void)
{
	CLKPR=0x80;
	CLKPR =0;

	DDRB = RELAY_PIN;
	PORTB = 0;
	
	Timer0Initialise();

	sei();

	while(1){
		PORTB = RELAY_PIN;		
		/* Wait for at least a few seconds */
		WaitSeconds(1);
		/* Wait for a random number of seconds. */
		WaitSeconds(1*(randomCounter & 0xf));
		
		PORTB = 0;
		
		/* Leave off by waiting 2 sec */
		WaitSeconds(2);
	}
}
