#ifndef TEHBUS_H_
#define TEHBUS_H_

#include "tehRAMS.h"
#include "tehSCREEN.h"
#include "tehBOOP.h"
#include "tehBEEP.h"

// We can have includes control this here, perhaps?
#include "chipperSDL.h"

class tehBUS {
private:
// This should be given to the class from somewhere else.
// tehCHIP would make some sense, but it feels messy.
    chipperSDL chipper_sdl;
    tehRAMS memory;
    tehSCREEN& screen;
    tehBEEP& speaker;
    tehBOOP& keyboard;

    bool speakerState;

public:
    tehBUS();
    
    bool get_exit_state();

    unsigned char read_ram(int addr);

    void write_ram(int addr, unsigned char val);

    void blank_screen();
    
    bool copy_sprite(int x, int y, short int addr, int len);

    void clock_bus();

    void screm();

    bool test_key(unsigned char value);

    unsigned char get_key();
    
};

#endif