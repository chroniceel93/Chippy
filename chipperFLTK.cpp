#include "chipperFLTK.h"

chipperFLTK::chipperFLTK() {
    this->main_window = new Fl_Window(340, 180);
    // this->box = new Fl_Box(20, 40, 300, 100, "Hello, World!");
    // this->box->box(FL_UP_BOX);
    // this->box->labelfont(FL_BOLD + FL_ITALIC);
    // this->box->labelsize(36);
    // this->box->labeltype(FL_SHADOW_LABEL);
    this->paws = new Fl_Button(0, 0, 100, 32, "Pause");
    this->paws->type(FL_TOGGLE_BUTTON);
    this->paws->callback(system_pause, static_cast<void*>(this));
    this->box = new Fl_Box(100, 0, 200, 32, "To_Init");
    this->main_window->end();
    this->main_window->show();
    this->is_exit = false;
    this->is_paused = false;
    return;
}

chipperFLTK::~chipperFLTK() {
    return;
}

void chipperFLTK::load_rom(std::string filename) {
    return;
}

void chipperFLTK::system_pause(Fl_Widget*, void* v) {
    chipperFLTK *temp = static_cast<chipperFLTK*>(v);
    temp->is_paused = true;
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
    if (Fl::check() == 0) {
        this->is_exit = true;
    }
    return;
}

bool chipperFLTK::get_exit_state() {
    return this->is_exit;
}