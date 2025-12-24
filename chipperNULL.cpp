#include "chipperNULL.h"

void chipperNULL::copy_screen(bool* data, int size) {
	return;
}

void chipperNULL::refresh_screen() {
	return;
}

void chipperNULL::set_resolution(int w, int h) {
	return;
}

int chipperNULL::get_width() {
	return 64;
}

int chipperNULL::get_height() {
	return 32;
}

void chipperNULL::SoundTick(bool mute) {
	return;
}

void chipperNULL::process_events() {
	return;
}

bool chipperNULL::get_exit_state() {
	return false;
}

bool chipperNULL::is_key_pressed(unsigned char value) {
	return false;
}

unsigned char chipperNULL::get_key_pressed() {
	return ' ';
}