#ifndef TEHBOOP_H_
#define TEHBOOP_H_

#include "SDL.h"

class tehBOOP {
private:
    // SDL_Event input; // input queue
    const Uint8 *state;
    bool exit;

    struct mapping {
        SDL_Scancode external_code;
    };

    mapping map[16] {
        SDL_SCANCODE_X, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, 
        SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_A, 
        SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_Z, SDL_SCANCODE_C, 
        SDL_SCANCODE_4, SDL_SCANCODE_R, SDL_SCANCODE_F, SDL_SCANCODE_V
    };

public:

    tehBOOP();

    void process_events();

    bool get_exit_state() const;

    bool is_key_pressed(unsigned char value) const;

    unsigned char get_key_pressed() const;
};


#endif