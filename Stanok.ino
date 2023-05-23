//C++ code
#include <util/delay.h>
#define F_CPU 16000000UL
#define default_time_step 50UL
#define DrumStepPin 2
#define GuideStepPin 3
#define GuideDirPin 4
#define DrumEnablePin 5
#define GuideEnablePin 6
#define LeftButtonPin 8
#define RightButtonPin 9

// Класс, содержащий шаги, счетчик шагов и период импульсов
class StepperPair{
	int Thickness = 40;		//Толщина проволоки, мм^(-1)
	unsigned long StepCount;			//Счетчик шагов
	int time_count = 12;	//Количество задержек 50мкс за полупериод одного шага
	void Step(int pin);
	void Step();
	
  public:
	void Forward(int l);
	void Backward(int l);
	void ResetPosition();
	void Off() {StepCount = 0; PORTD |= 1<<5; PORTD |= 1<<6;}
	
	void SetSpeed(unsigned long nu) {time_count = 500000UL / (nu * default_time_step);}
	// Вычисление задержек 50мкс полупериода одного шага
	// T = (10^6/(nu) * 1/2) / 50us;
	
	void SetThick(int t) {if (t >= 10 && t <= 100) Thickness = t;} // Установка толщины
} Dvig;

inline void BuffClear() // Очистка буфера Serial
{
	while(Serial.available())
		Serial.read();
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

void StepperPair :: Forward(int l) // Совместное движение с намоткой
{   
	int RatioInt = 200 / Thickness;
	int RatioRem = Thickness - 200 % Thickness;
	int TempRem = 0;

	PORTD &= ~(1 << DrumEnablePin);
	PORTD &= ~(1 << GuideEnablePin); // Включение драйверов

	PORTD |= 1 << GuideDirPin;
	_delay_us(1);  // Установка направления в правую сторону

	StepCount = l * 400; // Установка счетчика шагов: Шаги = Длина / 2мм * 800шаг

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

void StepperPair :: Backward(int l) // Совместное движение с намоткой влево
{   
	int RatioInt = 200 / Thickness;
	int RatioRem = Thickness - 200 % Thickness;
	int TempRem = 0;
  
	PORTD &= ~(1 << DrumEnablePin);
	PORTD &= ~(1 << GuideEnablePin); // Включение драйверов
  	
	PORTD &= ~(1<<4);
	_delay_us(1);  // Установка направления в левую сторону
  	
	StepCount = l * 400; // Установка счетчика шагов: Шаги = Длина / 2мм * 800шаг
  
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

void StepperPair :: ResetPosition() // Движение укладчика назад до упора
{   
	PORTD &= ~(1 << GuideEnablePin); // Включение драйвера
	
	PORTD &= ~(1 << GuideDirPin);
	_delay_us(1);  // Установка направления в левую сторону
	
	while(digitalRead(LeftButtonPin) == HIGH && digitalRead(GuideEnablePin) == LOW){ // Движение укладчика к левому концевику
		Step(GuideStepPin);
		CheckSerial();
	}
	PORTD |= 1 << GuideEnablePin; // Выключение драйвера
}

void CheckSerial()
{
	static bool ReadValue = false; // Флаг чтения числа
	
	String Command = "";  // Объявление строки, считывающей команду
	static int Value = 0;  // Объявление строки, считывающей значение
	
	if (Serial.available() >= 3 && !ReadValue){
		for (int i = 0; i < 3; ++i)
			Command += (char)Serial.read(); // Считывание трехсимвольной команды

		if (Command == "off"){
			Dvig.Off();
			ReadValue = false;
			Serial.println("Turning Off");
			BuffClear();
		}

		else if (Command == "ssp"){
			ReadValue = true;
			Serial.print("Setting speed: ");          
		}
		else
			BuffClear();
	}
	
	while(Serial.available() && ReadValue){
		char TempValue = (char) Serial.read();
		
		if (TempValue == '.'){
			Value = max(50,Value);
			Value = min(1500,Value);
		    	Dvig.SetSpeed(max(10,Value));
			BuffClear();
			Serial.println(Value);
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
}

void loop()
{	
	delay(100);
  
	String Command = "";  // Объявление строки, считывающей команду
	int Value = 0;  // Объявление строки, считывающей значение
	
	if (Serial.available() >= 3){
		for (int i = 0; i < 3; ++i)
			Command += (char)Serial.read();
	}
	// Считывание трехсимвольной команды
	
	if (Command == "rst"){
		Serial.println("Reseting position");
		BuffClear();
		Dvig.ResetPosition();
	}
	
	else if (Command == "bkw"){ 
		Serial.print("Moving Back, mm: ");
		while (Serial.available()){
			char TempValue = (char) Serial.read();
			
			if (TempValue == '.')
				break;
			
			if (TempValue < '0' || TempValue > '9'){
				Value = 0;
				BuffClear();
				Serial.println("Error");
				break;
			}
			Value *= 10;
			Value += TempValue - '0';
		}
		Serial.println(Value);
		BuffClear();
		Dvig.Backward(Value);
	} // Команда движения назад с намоткой на заданную длину
	
	else if (Command == "frw"){
		Serial.print("Moving forward, mm: ");
		while (Serial.available()){
			char TempValue = (char) Serial.read();
			
			if (TempValue == '.')
				break;
			
			if (TempValue < '0' || TempValue > '9'){
				Value = 0;
				BuffClear();
				Serial.println("Error");
				break;
			}
			Value *= 10;
			Value += TempValue - '0';
		}
		Serial.println(Value);
		BuffClear();
		Dvig.Forward(Value);
	} // Команда движения вперед с намоткой на заданную длину
	
	else if (Command == "ssp"){
		Serial.print("Setting speed: ");
		while (Serial.available()){
			char TempValue = (char) Serial.read();
			
			if (TempValue == '.')
				break;
			
			if (TempValue < '0' || TempValue > '9'){
				Value = 0;
				BuffClear();
				break;
			}
			
			Value *= 10;
			Value += TempValue - '0';            	
		} // Считывание остатка
		Value = max(50,Value);
		Value = min(1500,Value);
		Serial.println(Value);
		BuffClear();  
		Dvig.SetSpeed(Value);
	} // Команда установки скорости на заданное значение шаг/сек
	
	else if (Command == "sth"){
		Serial.print("Setting thickness: ");
		while (Serial.available()){
			char TempValue = (char) Serial.read();
			
			if (TempValue == '.')
				break;

			if (TempValue < '0' || TempValue > '9'){
				Value = 0;
				BuffClear();
				break;
			}
			
			Value *= 10;
			Value += TempValue - '0';            	
		} // Считывание остатка
		
		BuffClear();
		Serial.println(Value);
		Dvig.SetThick(Value);
	} // Команда установки толщины проволоки на заданное значение мм^-1
	else
		BuffClear();
}
