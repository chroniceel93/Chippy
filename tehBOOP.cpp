#include "tehBOOP.h"

tehBOOP::tehBOOP() {
    this->state = SDL_GetKeyboardState(NULL);
    this->exit = false;
}

void tehBOOP::process_events() {
    SDL_Event input;
    while (SDL_PollEvent(&input)) {
        if (input.type == SDL_QUIT) {
            this->exit = true;
        } // else do_nothing();
    }
    return;
}

bool tehBOOP::get_exit_state() const {
    return this->exit;
}

bool tehBOOP::is_key_pressed(unsigned char value) const {
    SDL_Scancode key = map[value].external_code;
    bool result = false;
    if (this->state[key] == 1) {
        result = true;
    }
    return result;
}

/**
 * @brief Retrieve the last pressed key.
 * 
 * Drops all but the latest input event from the queue, and then iterates over
 *   an internal array in SDL that holds the current keyboard state- The index
 *   i of map corresponds to the appropriate SDL scancode. The keyboard state
 *   array is indexed by these scancodes. If one of the keys we are watching is
 *   found- Whichever the first one is, as we do not care to handle multiple 
 *   concurrent inputs- We stop iterating, and return the pressed key code.
 * 
 * @return unsigned char - Detected key- 0x10 if no key pressed.
 */
unsigned char tehBOOP::get_key_pressed() const {
    unsigned char key_pressed = 0x10;
    for (auto i = 0; i < 0x10; i++) {
        if (this->state[map[i].external_code] == 1) {
            key_pressed = i;
            i = 0x10;
        } // else, do_nothing();
    }
    return key_pressed;
}