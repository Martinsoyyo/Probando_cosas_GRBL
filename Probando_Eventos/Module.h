#pragma once
#include <iostream>

enum _EVENT_ENUM {
    ON_MAIN_LOOP,
    ON_CONSOLE_LINE_RECEIVED,
    ON_GCODE_RECEIVED,
    ON_IDLE,
    ON_SECOND_TICK,
    ON_GET_PUBLIC_DATA,
    ON_SET_PUBLIC_DATA,
    ON_HALT,
    ON_ENABLE,
    NUMBER_OF_DEFINED_EVENTS
};

class Module;
using ModuleCallback = void (Module::*)(void*);
extern const ModuleCallback kernel_callback_functions[NUMBER_OF_DEFINED_EVENTS];

class Module
{
public:
    Module() {};

    virtual void on_main_loop(void*) {};
    virtual void on_console_line_received(void*) {};
    virtual void on_gcode_received(void*) {};
    virtual void on_idle(void*) {};
    virtual void on_second_tick(void*) {};
    virtual void on_get_public_data(void*) {};
    virtual void on_set_public_data(void*) {};
    virtual void on_halt(void*) {};
    virtual void on_enable(void*) {};

    void register_for_event(_EVENT_ENUM event_id);
};

