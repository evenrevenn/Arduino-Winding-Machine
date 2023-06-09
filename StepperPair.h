#ifndef STEPPERPAIR_H
#define STEPPERPAIR_H

class CommandTemplate;

// Класс, содержащий шаги, счетчик шагов и период импульсов
class StepperPair{
	int Thickness = 40;		    //Толщина проволоки, мм^(-2)
	unsigned long StepCount;    //Счетчик шагов
	int time_count = 12;	    //Количество задержек 50мкс за полупериод одного шага
	void Step(int pin);
	void Step();
	
public:
    void addCommand(CommandTemplate CommandType);
	CommandTemplate Commands[10];
};

#endif