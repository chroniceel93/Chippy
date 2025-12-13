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

// void tehBUS::set_resolution(int w, int h) {
//     this->screen.set_resolution(w, h);
//     this->screen_width = w;
//     this->screen_height = h;
//     return;
// }

// Why x + 6 - shift, when shift gets up to 7?
// x = 0. +6 = 6. 6-0 = 6. 6-1=5. 6-2=4. 6-3=3. 6-4=2. 6-5=1. 6-6=0.
// ... *OH WAIT*. len is variable. Duh.
// I was accidentally hardcoding the correct solution for a single case.
bool tehBUS::copy_sprite(int x, int y, short int addr, int len) {
    // If pixel doubling in effect - set scaling factor
    int scaling = this->framebuffer->get_video_mode() ? 1 : 2;

    int screen_width = this->framebuffer->get_framebuffer_width() / scaling;
    int screen_height = this->framebuffer->get_framebuffer_height() / scaling;

    int xpos = x;
    int ypos = y;

    if (x > (screen_width - 1)) {
        xpos %= screen_width;
        xpos /= scaling;
    }

    if (y > (screen_height - 1)) {
        ypos %= screen_height;
        ypos /= scaling;
    }

    unsigned char line;
    int shift = 0; // count how many times we've shifted.
    bool flipped = false;
    // for each byte -
    for (int i = 0; i < len ; i++) {
        shift = 7;
        // Read in the line
        line = this->memory->read_ram(addr+i);
        // Then iterate over the line, until there are no more high bits
        while (line > 0) { 
            // If the highest bit is high- Then it is part of the sprite
            if (line & 0x1) {
                if (this->framebuffer->draw_point(x + shift, y + i)) {
                    flipped = true;
                }
            }
            // Shift once, and loop.
            shift--;
            line = line >> 1;
        }
    }
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
