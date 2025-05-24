/**
 * @file chippySDL.h
 * @author William Tradewell
 * @brief SDL2 Implementation of Interfaces.
 * @version 0.2
 * @date 2025-05-23
 */

#include "tehSCREEN.h"
#include "tehBOOP.h"
#include "SDL.h"

#ifndef CHIPPERSDL_H_
#define CHIPPERSDL_H_

class chipperSDL: public tehSCREEN, public tehBOOP {
private:
    // Variables used for tehSCREEN
    bool SDL_Status; // Hold copy of SDL Status code.
    uint32_t pixels[2048]; // Array of pixels used to build display texture.
    SDL_Window *window; // Pointer to our window.
    SDL_Texture *render_texture; // Pointer to the render texture.
    SDL_Renderer *renderer; // Pointer to the renderer.
    SDL_Color background, foreground; // TODO: use these to replace hardcoded values
    SDL_Rect texrect;

    // Variables used for tehBOOP
    SDL_Event input; // input queue
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
    chipperSDL();
    ~chipperSDL();

    // Implemented from tehSCREEN
    void blank_screen();
    bool draw_point(int x, int y);
    void refresh_screen();

    // Implemented from tehBOOP
    virtual void process_events();
    virtual bool get_exit_state() const;
    virtual bool is_key_pressed(unsigned char value) const;
    virtual unsigned char get_key_pressed() const;
};

#endif