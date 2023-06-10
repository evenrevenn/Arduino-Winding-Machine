//C++ code
#include <util/delay.h>
#include "StepperPair.h" // Класс, содержащий шаги, счетчик шагов и период импульсов
#include "CommandTemplate.h"

#define F_CPU 16000000UL
#define default_time_step 50UL
#define DrumStepPin 2
#define GuideStepPin 3
#define GuideDirPin 4
#define DrumEnablePin 5
#define GuideEnablePin 6
#define LeftButtonPin 8
#define RightButtonPin 9

// Вычисление задержек 50мкс полупериода одного шага
// T = (10^6/(nu) * 1/2) / 50us;

inline void BuffClear() // Очистка буфера Serial
{
	while(Serial.available())
		Serial.read();
}

void CheckSerial()
{
	static bool ReadValue = false; 		// Флаг чтения числа
	
	static CommandTemplate *list = 0; 	// Объявление указателя на команду в списке команд
	String Key = "";  					// Объявление строки, считывающей команду
	static int Value = 0;  				// Объявление переменной, содержащей вводимое значение
	
	if (Serial.available() >= 3 && !ReadValue){
		for (int i = 0; i < 3; ++i)
			Key += (char)Serial.read(); // Считывание трехсимвольной команды

		for (list = Dvig.Commands; *list; list++){
			if (Key == list -> Key && list -> ReadValue){
				list -> function();
				BuffClear();
				return;
			}
			else if (Key == list -> Key && !(list -> ReadValue)){
				ReadValue = true;
				return;
			}
		}
		BuffClear();
	}
	while(Serial.available() && ReadValue){
		char TempValue = (char) Serial.read();
		
		if (TempValue == '.'){
		    list -> function(Value);
			BuffClear();
			ReadValue = false;
			Value = 0;
			break;
		}
		
		if (TempValue < '0' || TempValue > '9'){
		    Value = 0;
			ReadValue = false;
			BuffClear();
			Serial.println("Error");
		    break;
		}
		
		Value *= 10;
		Value += TempValue - '0';            			
	}
}

class ForwardCommand : public CommandTemplate{
	Key = "frw";
	Feedback = "Moving forward: ";
	ReadValue = true;
	Instant = false;
	void function(int i)	// Совместное движение с намоткой вправо
	{
		Serial.print(i);
		Serial.prinln(" mm");
		BuffClear();

		int RatioInt = 200 / Thickness;
		int RatioRem = Thickness - 200 % Thickness;
		int TempRem = 0;

		PORTD &= ~(1 << DrumEnablePin);
		PORTD &= ~(1 << GuideEnablePin); // Включение драйверов

		PORTD |= 1 << GuideDirPin;
		_delay_us(1);  // Установка направления в правую сторону

		StepCount = i * 400; // Установка счетчика шагов: Шаги = Длина / 2мм * 800шаг

		// Совместное движение укладчика и сердечника
		// Случай, когда толщина не кратна 2мм (например 0.3 мм)
		if (RatioRem){
			while(digitalRead(RightButtonPin) == HIGH && StepCount > 0 && digitalRead(GuideEnablePin) == LOW)
				{  	
				for (int i = 1; i < RatioInt; ++i)
					Step(2); // Отдельные шаги сердечника;
				TempRem += RatioRem;
				if (TempRem >= Thickness){
					Step();
				TempRem -= Thickness;
				}
				else{
					Step(2);
					Step(); // Одновременный шаг
				}
				CheckSerial();
			}
		}
		// Случай, когда толщина кратна 2мм (например 0.1 мм)
		else{
			while(digitalRead(RightButtonPin) == HIGH && StepCount > 0 && digitalRead(GuideEnablePin) == LOW)
			{  	
			for (int i = 1; i < RatioInt; ++i)
				Step(DrumStepPin); // Отдельные шаги сердечника;
			Step(); // Одновременный шаг
			CheckSerial();
			}
		}
		
		PORTD |= 1 << DrumEnablePin;
		PORTD |= 1 << GuideEnablePin; // Выключение драйверов
	}
};

class BackwardCommand : public CommandTemplate{
	Key = "bkw";
	Feedback = "Moving backward: ";
	ReadValue = true;
	Instant = false;
	void function(int i)	// Совместное движение с намоткой влево
	{
		Serial.print(i);
		Serial.prinln(" mm");
		BuffClear();

		int RatioInt = 200 / Thickness;
		int RatioRem = Thickness - 200 % Thickness;
		int TempRem = 0;
	
		PORTD &= ~(1 << DrumEnablePin);
		PORTD &= ~(1 << GuideEnablePin); // Включение драйверов
		
		PORTD &= ~(1<<4);
		_delay_us(1);  // Установка направления в левую сторону
		
		StepCount = i * 400; // Установка счетчика шагов: Шаги = Длина / 2мм * 800шаг
	
		// Совместное движение укладчика и сердечника
		
		if (RatioRem){
		//RatioInt++;
			while(digitalRead(LeftButtonPin) == HIGH && StepCount > 0 && digitalRead(GuideEnablePin) == LOW){
				for (int i = 1; i < RatioInt; ++i)
					Step(DrumStepPin); // Отдельные шаги сердечника;
				TempRem += RatioRem;
				if (TempRem >= Thickness){
					Step();
					TempRem -= Thickness;
				}
				else{
					Step(DrumStepPin);
					Step(); // Одновременный шаг
				}
				CheckSerial();
			}
		}
		else{
			while(digitalRead(LeftButtonPin) == HIGH && StepCount > 0 && digitalRead(GuideEnablePin) == LOW){
				for (int i = 1; i < RatioInt; ++i)
					Step(DrumStepPin); // Отдельные шаги сердечника;
				Step(); // Одновременный шаг
				CheckSerial();
			}
		}
		
		PORTD |= 1 << DrumEnablePin;
		PORTD |= 1 << GuideEnablePin; // Выключение драйверов
	}
};

class SetSpeedCommand : public CommandTemplate{
	Key = "ssp";
	Feedback = "Setting speed: ";
	ReadValue = true;
	Instant = true;
	void function(int i)	// Установка скорости
	{
		if (i >= 200 && i <= 1500){
			time_count = 500000UL / (i * default_time_step);
			Serial.println(i);
			BuffClear();
		}
	}
};

class SetThickCommand : public CommandTemplate{
	Key = "sth";
	Feedback = "Setting thickness: ";
	ReadValue = true;
	Instant = true;
	void function(int i)	// Установка толщины проволоки
	{
		if (i >= 10 && i <= 100){
			Thickness = i;
			Serial.print(i);
			BuffClear();
		}
	}
};

class OffCommand : public CommandTemplate{
	Key = "off";
	Feedback = "Turning off";
	ReadValue = false;
	Instant = true;
	void function(int i)	// Остановка двигателей
	{
		BuffClear();
		StepCount = 0;
		PORTD |= 1 << GuideEnablePin;
		PORTD |= 1 << DrumEnablePin;
	}
};

class ResetCommand : public CommandTemplate{
	Key = "rst";
	Feedback = "Resetting position";
	ReadValue = false;
	Instant = false;
	void function(int i)	// Возврат укладчика в начальное положение
	{
		BuffClear();

		PORTD &= ~(1 << GuideEnablePin); // Включение драйвера
		
		PORTD &= ~(1 << GuideDirPin);
		_delay_us(1);  // Установка направления в левую сторону
		
		while(digitalRead(LeftButtonPin) == HIGH && digitalRead(GuideEnablePin) == LOW){ // Движение укладчика к левому концевику
			Step(GuideStepPin);
			CheckSerial();
		}
		PORTD |= 1 << GuideEnablePin; // Выключение драйвера
	}
};

StepperPair Dvig; //Создание объекта для пары двигателей

void setup()
{
	Serial.begin(9600);
	
	pinMode(GuideStepPin, OUTPUT); // Step Укладчика
	pinMode(DrumStepPin, OUTPUT); // Step Сердечника
	
	pinMode(GuideStepPin, OUTPUT); // Enable Укладчика
	pinMode(DrumStepPin, OUTPUT); // Enable Сердечника
	PORTD |= 1 << GuideStepPin; 
	PORTD |= 1 << DrumStepPin; // Выключение драйверов
	
	pinMode(GuideDirPin, OUTPUT); // Dir Укладчика
	pinMode(RightButtonPin, INPUT_PULLUP); // Правый концевик
	pinMode(LeftButtonPin, INPUT_PULLUP); // Левый концевик

	// Добавление команд в список
	Dvig.addCommand(ForwardCommand f);
	Dvig.addCommand(BackwardCommand b);
	Dvig.addCommand(SetSpeedCommand s);
	Dvig.addCommand(SetThickCommand t);
	Dvig.addCommand(OffCommand o);
	Dvig.addCommand(ResetCommand r);
}

void loop()
{	
	delay(100);
	CheckSerial();
}
