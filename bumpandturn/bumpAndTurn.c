#include "avr/io.h"
#include "timer0.h"
#include "myinterrupts.h"

/*
	Bump and turn robot using 2 touch sensors and
	controlling 2 H Bridges

	ENABLE =enable, PD7
	MOTORLEFT0 =in1, PD6
	MOTORLEFT1 =in2, PD5
	MOTORRIGHT0 =in3, PB0
	MOTORRIGHT1 =in4, PB1
	TouchRight =right touch sensor, PD0
	TouchLeft =right touch sensor, PD1
*/

#define MOTORLEFT0	(1<<6)
#define MOTORLEFT1	(1<<7)

#define MOTORRIGHT0	(1<<1)
#define MOTORRIGHT1	(1<<2)

#define SwitchRight	(1<<3)
#define SwitchLeft	(1<<4)

void LEFTBACKWARDS(void)
{
	PORTD |=MOTORLEFT1;
	PORTD &=~MOTORLEFT0;
}

void LEFTFORWARDS(void)
{
	PORTD |=MOTORLEFT0;
	PORTD &=~MOTORLEFT1;
}

void RIGHTOFF(void)
{
	PORTB &=~MOTORRIGHT1;
	PORTB &=~MOTORRIGHT0;
}


void RIGHTFORWARDS(void)
{
	PORTB |=MOTORRIGHT1;
	PORTB &=~MOTORRIGHT0;
}

void RIGHTBACKWARDS(void)
{
	PORTB &=~MOTORRIGHT1;
	PORTB |=MOTORRIGHT0;
}
void LEFTOFF(void)
{
	PORTD &=~MOTORLEFT0;
	PORTD &=~MOTORLEFT1;
}


void BackUpAndTurn(char x)
{
	LEFTBACKWARDS();
	RIGHTBACKWARDS();
	WaitMilliseconds(150);
	/*if (x='L')
	{
		RIGHTFORWARDS();
	}
	else if (x='R')
	{
		LEFTFORWARDS();
	}
	WaitMilliseconds(30);
	RIGHTOFF();
	LEFTOFF();
	*/
}

int main(void)
{


	DDRD &= ~(SwitchLeft | SwitchRight); // Set to these pins to inputs
	PORTD = SwitchLeft | SwitchRight;     // Set these pins to pull-up

	DDRD |= MOTORLEFT0;
	DDRD |= MOTORLEFT1;
	DDRB |= MOTORRIGHT0;
	DDRB |= MOTORRIGHT1;
	DDRB |= (1<<0);
	Timer0Initialise();

	interrupt_Enable(1);


	int touchRight =0,touchLeft=0,R=0,L=0,x=0;
	while(1){
	for (x=0;x<10000;x++)
	{
		touchRight=(PIND &SwitchLeft);
		touchLeft=(PIND &SwitchRight);
		if (touchLeft)
		{
			
			LEFTFORWARDS();
			L=0;
		}
		else 
		{
			L++;
			if (L>=1000)
			{
				BackUpAndTurn('L');
				LEFTFORWARDS();
				WaitMilliseconds(30);
				RIGHTOFF();
				LEFTOFF();
			}
		}
		if (touchRight)
		{
			RIGHTFORWARDS();
			
		}
		else
		{
			R++;
			if (R>=1000)
			{
				BackUpAndTurn('R');
				RIGHTFORWARDS();
				WaitMilliseconds(30);
				RIGHTOFF();
				LEFTOFF();PORTB |=(1<<0);
			}
		}

		}
		PORTB ^=(1<<0);
	}
}

