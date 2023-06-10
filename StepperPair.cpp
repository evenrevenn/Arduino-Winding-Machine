#include "StepperPair.h"
#include "CommandTemplate.h"
#include <util/delay.h>

void addCommand(CommandTemplate AddedCommand)
{
	CommandTemplate *list;
	for (list = Commands; *list; list++);

	*list = AddedCommand;
}

void StepperPair :: Step(int pin) // Функция шага отдельного двигателя
{  
	int count;
	PORTD |= 1<<pin;
	for (count = time_count; count > 0; count--)
		_delay_us(default_time_step);
	
	PORTD &= ~(1<<pin);
	for (count = time_count; count > 0; count--)
		_delay_us(default_time_step);
}

void StepperPair :: Step() // Функция одновременного шага обоих двигателей
{   
	int count;
	PORTD |= 1 << GuideStepPin;
	PORTD |= 1 << DrumStepPin;
	for (count = time_count; count > 0; count--)
		_delay_us(default_time_step);

	PORTD &= ~(1 << GuideStepPin);
	PORTD &= ~(1 << DrumStepPin);
	for (count = time_count; count > 0; count--)
		_delay_us(default_time_step);
  
	StepCount--;
}