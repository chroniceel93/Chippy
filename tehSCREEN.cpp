#include "tehSCREEN.h"

tehSCREEN::tehSCREEN()  {
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
    return;
}

tehSCREEN::~tehSCREEN() {
    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyWindow(this->window);
    SDL_DestroyTexture(this->render_texture);
    this->renderer = NULL;
    this->window = NULL;
    this->render_texture = NULL;
    SDL_Quit();
    return;
}


/**
 * @brief Why is this in its own function?
 * 
 * Take raw pixel data, and replace a texture.
 * 
 */
void tehSCREEN::build_tex() {
    // Normally you would want to stream the texture, but I found that there's
    // quite a lot more overhead with textures this small, than simply updating
    // it.
    this->texrect.x = 0;
    this->texrect.y = 0;
    this->texrect.w = 64;
    this->texrect.h = 32;
    SDL_UpdateTexture(this->render_texture, NULL, this->pixels, 4 * 64);
    return;
}

/**
 * @brief Sets every pixel to opaque black. This will clear the screen.
 * 
 */
void tehSCREEN::blank_screen()  {
    for (int i = 0; i < 2048; i++) {
        this->pixels[i] = 0x000000FF;
    }
    this->build_tex();
    return;
}

/**
 * @brief XOR one pixel, and return whether any white pixels were flipped.
 * 
 * The coordinate plane starts at the top left corner. X increases to the right
 *   and Y increases downwards. To evaluate a given pixel, the coordinates must
 *   be converted into an array index. This pixel offset can be found by mult-
 *   iplying the Y-coordinate by the number of pixels per row- And then adding
 *   to that the value of the X-coordinate. More specifically, for the classic
 *   display mode, this works out to (y * 64) + x.
 * 
 * Then, we test the value of the pixel in that address. If it is all hi- Then 
 *   we need to set our return variable to true, and flip it to black. Otherwise,
 *   we set the pixel to white. We do not have, or need, logic to explicitly set
 *   a pixel to black.
 * 
 * NOTICE:
 * 
 * This effect seems to misbehave on MacOS. Looking into it further. It looks 
 *   like textures are being blitted to the render texture out of order. This 
 *   leads to a rather jarring jittering effect, displaying screen data that can 
 *   be several seconds old.
 * 
 * @param x - X coordinate
 * @param y  - Y coordinate
 * @return true - White pixel was flipped
 * @return false - White pixel was not flipped.
 */
bool tehSCREEN::draw_point(int x, int y) {
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


/**
 * @brief Updates the renderer, and presents it.
 * 
 */
void tehSCREEN::refresh_screen()  {
    this->update_screen();
    SDL_RenderPresent(this->renderer);
    return;
}

/**
 * @brief Updates the render texture, and blends it with the renderer.
 * 
 * The values for srcrect and dstrect are currently hardcoded. The blending mode
 *   is set to blend by alpha values. Black pixels in the render texture are
 *   treated as transparent. This provides a nice, fade-out effect for white
 *   pixels.
 * 
 * TODO: Set dstrect = window dimensions.
 */
void tehSCREEN::update_screen() {
    this->build_tex();
    SDL_Rect dstrect;
    dstrect.x = 0;
    dstrect.y = 0;
    dstrect.w = 512;
    dstrect.h = 256;
    SDL_RenderCopy(this->renderer, this->render_texture, &this->texrect, &dstrect);
    return;
}

// A good bit of restructuring must happen if we want to handle multiple
//  resolutions.