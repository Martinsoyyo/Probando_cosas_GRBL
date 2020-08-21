#pragma once
#include "Kernel.h"
//#include "Module.h"

class Serial :public Module
{
public:
	Serial() {};

	virtual void on_main_loop(void*) {
		std::cout << "[in] <SERIAL> void on_main_loop(void*) " << std::endl;
		
		std::string _message = "para del GCODE";
		THEKERNEL->call_event(ON_CONSOLE_LINE_RECEIVED, &_message);
		std::cout << _message << std::endl;
		std::cout << "[out] <SERIAL> void on_main_loop(void*) " << std::endl;
	};

};

