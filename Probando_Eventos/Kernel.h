#pragma once
#include <vector>
#include <array>

#include "Module.h"	

#define THEKERNEL Kernel::instance

class Kernel{
public:
	static Kernel* instance; // the Singleton instance of Kernel usable anywhere
	void register_for_event(_EVENT_ENUM id_event, Module* module);
	void call_event(_EVENT_ENUM, void*);
private:

	Kernel() {};
	std::array<std::vector<Module*>, NUMBER_OF_DEFINED_EVENTS> hooks;
};
