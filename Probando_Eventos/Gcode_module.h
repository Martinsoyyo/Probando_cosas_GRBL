#pragma once
#include "Kernel.h"
#include "Module.h"

class Gcode_module :public Module
{
public:
	Gcode_module() {};

	virtual void on_console_line_received(void* MESSAGE) {
		std::cout << "[in]<Gcode_module> void on_console_line_received(void*) " << std::endl;
	
		std::string _message = std::move(*static_cast<std::string*>(MESSAGE));
		std::cout << _message << std::endl;

		std::cout << "[out]<Gcode_module> void on_console_line_received(void*) " << std::endl;
	};
};

