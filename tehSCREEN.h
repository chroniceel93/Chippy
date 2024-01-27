/**
 * @file tehSCREEN.h
 * @author William Tradewell
 * @brief SDL2 Display Class.
 * @version 0.1
 * @date 2023-11-04
 */

#ifndef TEHSCREEN_H_
#define TEHSCREEN_H_

#include "SDL.h"

/**
 * @brief TehSCREEN instances, and performs all operations on, our renderer.
 * 
 * In summary, this class is currently extremely barebones- Implementing Just
 *   Enough boilerplate to provide a display area, and error checking. 
 *   As such, this class is currently very, very limited. We create a single, 
 *   hardware-accelerated window, with a fixed resolution.
 */
class tehSCREEN {
private:
    bool SDL_Status; // Hold copy of SDL Status code.
    uint32_t pixels[2048]; // Array of pixels used to build display texture.
    SDL_Window *window; // Pointer to our window.
    SDL_Texture *render_texture; // Pointer to the render texture.
    SDL_Renderer *renderer; // Pointer to the renderer.
    SDL_Color background, foreground; // TODO: use these to replace hardcoded values
    SDL_Rect texrect;

public:

    void build_tex();

    void blank_screen();

    tehSCREEN();

    bool draw_point(int x, int y);

    // bool update_state();

    void refresh_screen();

    void update_screen();

    ~tehSCREEN();
};

#endif