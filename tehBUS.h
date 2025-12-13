#ifndef TEHBUS_H_
#define TEHBUS_H_

#include "tehCOMMONZ.h"
#include "tehRAMS.h"
#include "tehVIDEO.h"
#include "tehSCREEN.h"
#include "tehBOOP.h"
#include "tehBEEP.h"

class tehBUS {
private:
// This should be given to the class from somewhere else.
// tehCHIP would make some sense, but it feels messy.
    // chipperSDL chipper_sdl;
    tehRAMS* memory;
    tehVIDEO* framebuffer;
    tehSCREEN& screen;
    tehBEEP& speaker;
    tehBOOP& keyboard;
    bool speakerState;

    chippy::systype system;

public:
    tehBUS(tehSCREEN& s, tehBEEP& b, tehBOOP& k, chippy::systype sys = chippy::CHIP8);
    
    void clock_bus();
    bool get_exit_state();

    // Memory
    unsigned char read_ram(int addr);
    void write_ram(int addr, unsigned char val);

    // Video
    void blank_screen();
    void set_video_mode(bool mode);
    bool copy_sprite(int x, int y, short int addr, int len);

    // Input
    unsigned char get_key();
    bool test_key(unsigned char value);

    // Audio
    void screm();
};

#endif