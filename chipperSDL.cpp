#include "chipperSDL.h"

chipperSDL::chipperSDL() {
    this->SDL_Status = true; // Assume SDL is good- Set to false if init fails
    this->background.r = 0;
    this->background.g = 0;
    this->background.b = 0;
    this->foreground.r = 255;
    this->foreground.g = 255;
    this->foreground.b = 255;

    this->texrect.x = 0;
    this->texrect.y = 0;
    this->texrect.w = 64;
    this->texrect.h = 32;

    // init SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw SDL_GetError();
        this->SDL_Status = false;
    } else {
        // create window
        this->window = SDL_CreateWindow(
            "Chip-8"
            , SDL_WINDOWPOS_CENTERED
            , SDL_WINDOWPOS_CENTERED
            , 512
            , 256
            , SDL_WINDOW_SHOWN
        );
        // if the SDL_CreateWindow fails, then the pointer *window will be
        // null. So, if the window == nullptr, then throw an error
        if (this->window == NULL) {
            throw SDL_GetError();
            this->SDL_Status = false;
        } else {
            // Attempt to create a renderer
            // Keep flags at 0 - SDL selects hardware renderers first, and
            //   falls back to Software if not available. 
            this->renderer = SDL_CreateRenderer(
                this->window
                , -1
                , 0 
            );
            // Repeat the nullptr test with renderer
            if (this->renderer == NULL) {
                throw SDL_GetError();
                this->SDL_Status = false;
            } else {
                // Set the default drawing color
                SDL_SetRenderDrawColor(
                    this->renderer
                    , this->background.r
                    , this->background.g
                    , this->background.b
                    , 255
                );

                // Initialize the render texture
                this->render_texture = SDL_CreateTexture(
                    this->renderer
                    , SDL_PIXELFORMAT_RGBA8888
                    , SDL_TEXTUREACCESS_STREAMING
                    , 64
                    , 32
                );

                if (this->render_texture == NULL) {
                    throw SDL_GetError();
                }

                SDL_SetTextureBlendMode(this->render_texture, SDL_BLENDMODE_BLEND);

                this->blank_screen();

                //Present the renderer
                SDL_RenderPresent(this->renderer);
            }
        }
    }

    // Init SDL input variables.
    this->state = SDL_GetKeyboardState(NULL);
    this->exit = false;
    return;
}

chipperSDL::~chipperSDL() {
    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyWindow(this->window);
    SDL_DestroyTexture(this->render_texture);
    this->renderer = NULL;
    this->window = NULL;
    this->render_texture = NULL;
    SDL_Quit();
    return;
}

void chipperSDL::blank_screen() {
    for (int i = 0; i < 2048; i++) {
        this->pixels[i] = 0x000000FF;
    }
    return;
}

bool chipperSDL::draw_point(int x, int y) {
        bool flipped = false;
    // Logic for if clipping is off.
    // if (x > 63) {
    //     x = x % 64;
    // }

    // if (y > 31) {
    //     y = y % 32;
    // }

    int pixel_offset = (y * 64) + x;

    // if (pixel_offset > 2047) {
    //     pixel_offset = pixel_offset % 2048;
    // }
    if ((x < 64) && (y < 32)) { // test if clipping is on
        // Pixel data is 0xRRGGBBAA
        if (this->pixels[pixel_offset] == 0xFFFFFFFF) {
// The 0x31 alpha value was chosen more or less at random. I just needed the 
// black pixels to have some transparency so that, when this new texture is 
// blended with the render copy, white pixels will slowly fade out. This
// provides us our screen-burn effect!
            this->pixels[pixel_offset] = 0x00000031;
            flipped = true;
        } else {
            this->pixels[pixel_offset] = 0xFFFFFFFF;
        }
    } // else, do_nothing();
    return flipped;
}

void chipperSDL::refresh_screen() {
    // Set texture dimensions
    this->texrect.x = 0;
    this->texrect.y = 0;
    this->texrect.w = 64;
    this->texrect.h = 32;
    // Update texture with updated values
// Normally you would want to stream the texture, but I found that there's
// quite a lot more overhead with textures this small, than simply updating
// it.    
    SDL_UpdateTexture(this->render_texture, NULL, this->pixels, 4 * 64);
    // Create rect for blit
    SDL_Rect dstrect;
    dstrect.x = 0;
    dstrect.y = 0;
    dstrect.w = 512;
    dstrect.h = 256;
    // Copy texture to the renderer and present
    SDL_RenderCopy(this->renderer, this->render_texture, &this->texrect, &dstrect);
    SDL_RenderPresent(this->renderer);
}

// Implemented from tehBOOP
void chipperSDL::process_events() {
    SDL_Event input;
    while (SDL_PollEvent(&input)) {
        if (input.type == SDL_QUIT) {
            this->exit = true;
        } // else do_nothing();
    }
    return;
}

bool chipperSDL::get_exit_state() const {
    return this->exit;
}

bool chipperSDL::is_key_pressed(unsigned char value) const {
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
unsigned char chipperSDL::get_key_pressed() const {
    unsigned char key_pressed = 0x10;
    for (auto i = 0; i < 0x10; i++) {
        if (this->state[map[i].external_code] == 1) {
            key_pressed = i;
            i = 0x10;
        } // else, do_nothing();
    }
    return key_pressed;
}