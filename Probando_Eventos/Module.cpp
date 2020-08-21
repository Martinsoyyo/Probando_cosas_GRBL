#include "Module.h"
#include "Kernel.h"

const ModuleCallback kernel_callback_functions[NUMBER_OF_DEFINED_EVENTS] = {
    &Module::on_main_loop,
    &Module::on_console_line_received,
    &Module::on_gcode_received,
    &Module::on_idle,
    &Module::on_second_tick,
    &Module::on_get_public_data,
    &Module::on_set_public_data,
    &Module::on_halt,
    &Module::on_enable,
};

void Module::register_for_event(_EVENT_ENUM event_id) {
    THEKERNEL->register_for_event(event_id, this);
}
