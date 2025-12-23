#include "tehBUS.h"

tehBUS::tehBUS(tehSCREEN& s, tehBEEP& b, tehBOOP& k, chippy::systype sys) 
                 : screen(s)
                 , keyboard(k)
                 , speaker(b) {
    this->system = sys;
    this->memory = new tehRAMS();
    this->framebuffer = new tehVIDEO(s, sys);
    this->speakerState = true; // start muted
}

void tehBUS::clock_bus() {
    this->keyboard.process_events();
    this->framebuffer->update_screen();
    // this->screen.refresh_screen();
    this->speaker.SoundTick(this->speakerState);
    this->speakerState = true;
    return;
}

bool tehBUS::get_exit_state() {
    return this->keyboard.get_exit_state();
}

unsigned char tehBUS::read_ram(int addr) {
    return this->memory->read_ram(addr);
}

void tehBUS::write_ram(int addr, unsigned char val) {
    this->memory->write_ram(addr, val);
    return;
}

void tehBUS::blank_screen() {
    this->framebuffer->blank_screen();
    return;
}

void tehBUS::set_video_mode(bool mode) {
    this->framebuffer->set_video_mode(mode);
}

bool tehBUS::copy_sprite(int x, int y, short int addr, int len) {
    bool flipped = false;
    // Copy memory containing sprite into bool array
    unsigned char sprite[32] = {0};
    // if length is 16, we're always copying 32 bytes of memory. This is a
    // special case for SUPERCHIP's 16x16 sprite drawing
    int length = (len == 16) ? 32 : len;
    for (int i = 0; i < length ; i++) {
        sprite[i] = this->memory->read_ram(addr+i);
    }

    flipped = this->framebuffer->draw_sprite(x, y, len, sprite);
    return flipped;
}

unsigned char tehBUS::get_key() {
    return this->keyboard.get_key_pressed();
}

bool tehBUS::test_key(unsigned char value) {
    return this->keyboard.is_key_pressed(value);
}

void tehBUS::screm() {
    this->speakerState = false;
}
