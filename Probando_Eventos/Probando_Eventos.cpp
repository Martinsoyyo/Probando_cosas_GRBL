#include <string>
#include <iostream>
#include <tuple>
#include <signal.h>
#include <chrono>
#include <windows.h>
#include <functional>
#include "queue.h"
#include <string>
#include <algorithm>

#include "Kernel.h"
#include "Serial.h"
#include "Gcode_module.h"

class MIO {
public:
	char _last[12];
	std::string _name;
	uint32_t _num;
	float _code;

	MIO() {};
	MIO(char* LAST, std::string NAME, uint32_t NUM, float CODE) :
		_name(NAME), _num(NUM), _code(CODE)
	{
		memcpy(_last, LAST, sizeof(_last));
	};

	void foo(MIO&& OBJ) {
		std::cout << " foo()" << std::endl;
		*this = OBJ;
	}

};

int main() {
	char KIKO[] = "kiko";
	MIO A{ KIKO,"prueba",23,9.13f };

	MIO B;

	Serial _serial;
	_serial.register_for_event(ON_MAIN_LOOP);

	Gcode_module _gcode;
	_gcode.register_for_event(ON_CONSOLE_LINE_RECEIVED);


	while (1) {
		THEKERNEL->call_event(ON_MAIN_LOOP, nullptr);
		Sleep(2000);
	}

	return 0;
}