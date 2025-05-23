#include "tehSCREEN.h"
#include "SDL.h"

#ifndef CHIPPERSDL_H_
#define CHIPPERSDL_H_

class chipperSDL: public tehSCREEN {
private:
    bool SDL_Status; // Hold copy of SDL Status code.
    uint32_t pixels[2048]; // Array of pixels used to build display texture.
    SDL_Window *window; // Pointer to our window.
    SDL_Texture *render_texture; // Pointer to the render texture.
    SDL_Renderer *renderer; // Pointer to the renderer.
    SDL_Color background, foreground; // TODO: use these to replace hardcoded values
    SDL_Rect texrect;

public:
    chipperSDL();
    ~chipperSDL();

    // Implemented from tehSCREEN

    void blank_screen();
    bool draw_point(int x, int y);
    void refresh_screen();
};

#endif