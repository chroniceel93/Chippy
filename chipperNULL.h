

#ifndef CHIPPERNULL_H_
#define CHIPPERNULL_H_

#include "tehSCREEN.h"
#include "tehBOOP.h"
#include "tehBEEP.h"
#include "tehGUI.h"

class chipperNULL: public tehSCREEN, public tehBOOP, public tehBEEP, public tehGUI {
public:
	void copy_screen(bool* data, int size);
	void refresh_screen();
	void set_resolution(int w, int h);
	int get_width();
	int get_height();

	void copy_audio(uint8_t* data, int size);
	int get_sample_rate();
	int get_bytes_per_sample();
	int get_buffer_size();

	void process_keypad_events();
	bool get_exit_state() const;
	bool is_key_pressed(unsigned char value) const;
	unsigned char get_key_pressed() const;

	void load_rom(std::string filename);
	void system_pause();
	void system_resume();
	chippy::systype get_quirks_mode();
	void set_quirks_mode(chippy::systype quirks_mode);
	int get_interpreter_speed();
	void set_interpreter_speed(int interpreter_speed);
	void process_gui_events();
	bool get_exit_state();
};

#endif