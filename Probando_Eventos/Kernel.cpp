#include "Kernel.h"

Kernel* Kernel::instance = new Kernel;

void Kernel::register_for_event(_EVENT_ENUM id_event, Module* mod) {
    this->hooks[id_event].push_back(mod);
}

void Kernel::call_event(_EVENT_ENUM id_event, void* argument) {
    // send to all registered modules
    for (auto m : hooks[id_event]) {
        (m->*kernel_callback_functions[id_event])(argument);
    }
};