#include "tehBUS.h"

tehBUS::tehBUS(tehSCREEN& s, tehBEEP& b, tehBOOP& k) 
                 : screen(s)
                 , keyboard(k)
                 , speaker(b) {
    this->speakerState = true; // start muted
    this->screen_width = this->screen.get_width();
    this->screen_height = this->screen.get_height();
}

void tehBUS::clock_bus() {
    this->keyboard.process_events();
    this->screen.refresh_screen();
    this->speaker.SoundTick(this->speakerState);
    this->speakerState = true;
    return;
}

bool tehBUS::get_exit_state() {
    return this->keyboard.get_exit_state();
}

unsigned char tehBUS::read_ram(int addr) {
    return this->memory.read_ram(addr);
}

void tehBUS::write_ram(int addr, unsigned char val) {
    this->memory.write_ram(addr, val);
    return;
}

void tehBUS::blank_screen() {
    this->screen.blank_screen();
    return;
}

void tehBUS::set_resolution(int w, int h) {
    this->screen.set_resolution(w, h);
    this->screen_width = w;
    this->screen_height = h;
    return;
}

// Why x + 6 - shift, when shift gets up to 7?
// x = 0. +6 = 6. 6-0 = 6. 6-1=5. 6-2=4. 6-3=3. 6-4=2. 6-5=1. 6-6=0.
// ... *OH WAIT*. len is variable. Duh.
// I was accidentally hardcoding the correct solution for a single case.
bool tehBUS::copy_sprite(int x, int y, short int addr, int len) {
    if (x > (this->screen_width - 1)) {
        x %= this->screen_width;
    }

    if (y > (this->screen_height - 1)) {
        y %= this->screen_height;
    }
    unsigned char line;
    int shift = 0; // count how many times we've shifted.
    bool flipped = false;
    // for each byte -
    for (int i = 0; i < len ; i++) {
        shift = 7;
        // Read in the line
        line = this->memory.read_ram(addr+i);
        // Then iterate over the line, until there are no more high bits
        while (line > 0) { 
            // If the highest bit is high- Then it is part of the sprite
            if (line & 0x1) {
                if (this->screen.draw_point(x + shift, y + i)) {
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
