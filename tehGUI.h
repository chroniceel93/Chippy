#ifndef TEHGUI_H_
#define TEHGUI_H_

#include "tehCOMMONZ.h"

#include <string>

class tehGUI {
public:
    virtual ~tehGUI() {}

    virtual void load_rom(std::string filename) = 0;

    virtual void system_pause() = 0;
    virtual void system_resume() = 0;

    virtual chippy::systype get_quirks_mode() = 0;
    virtual void set_quirks_mode(chippy::systype quirks_mode) = 0;

    virtual int get_interpreter_speed() = 0;
    virtual void set_interpreter_speed(int interpreter_speed) = 0;

    virtual void process_gui_events() = 0;
    virtual bool get_exit_state() = 0;
};

/**
 * Stuff we need:
 * 1. ROM loading - And re-loading. Switch ROM in Execution
 *      - Recent history.
 * 2. System Reset
 * 3. System Pause
 * 4. Get/Set quirks Mode
 * 5. Get/Set interpreter speed
 * Stretch goals:
 * 1. Get/Set CPU state
 * 2. Get/Set RAM state - Possibly for debugger?
 */

#endif