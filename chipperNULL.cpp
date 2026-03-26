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

void chipperNULL::copy_audio(uint8_t* data, int size) {
	return;
}

int chipperNULL::get_sample_rate() {
	return 48000; // use sane default so math doesn't blow up
}

int chipperNULL::get_bytes_per_sample() {
	return 2;
}

int chipperNULL::get_buffer_size() {
	return 0; 
}

void chipperNULL::process_events() {
	return;
}

bool chipperNULL::get_exit_state() const {
	return false;
}

bool chipperNULL::is_key_pressed(unsigned char value) const {
	return false;
}

unsigned char chipperNULL::get_key_pressed() const {
	return ' ';
}