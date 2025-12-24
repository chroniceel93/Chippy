

#ifndef CHIPPERNULL_H_
#define CHIPPERNULL_H_

#include "tehSCREEN.h"
#include "tehBOOP.h"
#include "tehBEEP.h"

class chipperNULL: public tehSCREEN, public tehBOOP, public tehBEEP {
public:
	void copy_screen(bool* data, int size);
	void refresh_screen();
	void set_resolution(int w, int h);
	int get_width();
	int get_height();

	void SoundTick(bool mute);

	void process_events();
	bool get_exit_state();
	bool is_key_pressed(unsigned char value);
	unsigned char get_key_pressed();
};

#endif