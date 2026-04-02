#include "chipperFLTK.h"

chipperFLTK::chipperFLTK() {
    return;
}

chipperFLTK::~chipperFLTK() {
    return;
}

void chipperFLTK::load_rom(std::string filename) {
    return;
}

void chipperFLTK::system_pause() {
    return;
}

void chipperFLTK::system_resume() {
    return;
}

chippy::systype chipperFLTK::get_quirks_mode() {
    return chippy::CHIP8;
}

void chipperFLTK::set_quirks_mode(chippy::systype quirks_mode) {
    return;
}

int chipperFLTK::get_interpreter_speed() {
    return 0;
}

void chipperFLTK::set_interpreter_speed(int interpreter_speed) {
    return;
}

void chipperFLTK::process_gui_events() {
    return;
}

bool chipperFLTK::get_exit_state() {
    return false;
}