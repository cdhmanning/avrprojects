#include "avr/io.h"
#include "timer0.h"
#include "myinterrupts.h"
#include "servo.h"

int main(void)
{

	Timer0Initialise();
	servo_Initialise();

	interrupt_Enable(1);

	while(1){
		servo_Set(0,100);
		WaitMilliseconds(2000);
		servo_Set(0,150);
		WaitMilliseconds(1000);
		servo_Set(0,200);
		WaitMilliseconds(1000);
	}		
}

