#include "chipperSDL3.h"

// Init SDL

bool chipperSDL3::init_SDL() {
    bool result = true;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == false) {
        throw SDL_GetError();
        result = false;
    } else {
        if (this->init_SDL_window()) {
            if (this->init_renderer()) {
                if (!this->init_textures()) result = false;
            } else {
                result = false;
            }

            this->init_SDL_Audio();

        } else {
            result = false;
        }
    }
    return result;
}

bool chipperSDL3::init_SDL_Audio() {
    bool result = true;

    // We fill audioSettings with the values we want, and pass that to SDL.
    // Once SDL's opened the audio device, it replaces these values with
    // what it is actually using.
    const SDL_AudioSpec spec = { SDL_AUDIO_S16, 2 ,this->samplesPerSecond };
    this->audioStream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK
        , &spec
        , NULL
        , NULL
    );
    // TODO: verify this is working as you'd expect.
    return result;
}

bool chipperSDL3::init_SDL_window() {
    this->window_width = chippy::DEFAULT_WINDOW_WIDTH;
    this->window_height = chippy::DEFAULT_WINDOW_HEIGHT;
    bool result = true;
    // Create Window
    this->window = SDL_CreateWindow(
        "Chip-8"
        , this->window_width
        , this->window_height
        , SDL_WINDOW_RESIZABLE
    );
    // if the SDL_CreateWindow fails, then the pointer *window will be null. 
    // So, if the window == nullptr, then throw an error
    if (this->window == NULL) {
        throw SDL_GetError();
        result = false;
    } else {
        this->is_mouse_focus = true;
        this->is_keyboard_focus = true;

    }
    return result;
}

bool chipperSDL3::init_renderer() {
    bool result = true;
    // Attempt to create a renderer
    // Keep flags at 0 - SDL selects hardware renderers first, and falls back to
    // software if not available.
    this->renderer = SDL_CreateRenderer(
        this->window
        , NULL
    );

    if (this->renderer == NULL) {
        throw SDL_GetError();
        result = false;
    }

    return result;
}

bool chipperSDL3::init_textures() {
    bool result = true;

    this->render_texture = SDL_CreateTexture(
        this->renderer
        , SDL_PIXELFORMAT_RGBA8888
        , SDL_TEXTUREACCESS_STREAMING
        , this->vbuf_w
        , this->vbuf_h
    );

    SDL_SetTextureBlendMode(this->render_texture, SDL_BLENDMODE_BLEND);

    this->fade_texture = SDL_CreateTexture(
        this->renderer
        , SDL_PIXELFORMAT_RGBA8888
        , SDL_TEXTUREACCESS_TARGET
        , this->vbuf_w
        , this->vbuf_h
    );

    // If either texture is null, something is wrong. Throw an error and set
    // result to false.
    if (this->render_texture == NULL || this->fade_texture == NULL) {
        throw SDL_GetError();
        result = false;
    } else {
        SDL_SetTextureScaleMode(this->render_texture, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureScaleMode(this->fade_texture, SDL_SCALEMODE_NEAREST);
    }

    return result;
}

void chipperSDL3::delete_textures() {
    SDL_DestroyTexture(this->render_texture);
    SDL_DestroyTexture(this->fade_texture);
    this->render_texture = NULL;
    this->fade_texture = NULL;
    return;
}

void chipperSDL3::init_pixel_array() {
    int array_size = sizeof(uint32_t) * this->vbuf_h * this->vbuf_w;
    this->pixel_array = (uint32_t*) SDL_malloc(array_size);
    return;
}

void chipperSDL3::delete_pixel_array() {
    SDL_free(this->pixel_array);
    this->pixel_array = NULL;
    return;
}

chipperSDL3::chipperSDL3() {
    this->SDL_Status = true; // Assume SDL is good- Set to false if init fails
    this->background.r = 0;
    this->background.g = 0;
    this->background.b = 0;
    this->foreground.r = 255;
    this->foreground.g = 255;
    this->foreground.b = 255;

    // Sane defaults, but expect these to be overidden later on
    this->vbuf_w = 64;
    this->vbuf_h = 32;

    // We're setting the default dimensions to match the Chip-8 video buffer.
    this->texrect.x = 0;
    this->texrect.y = 0;
    this->texrect.w = this->vbuf_w;
    this->texrect.h = this->vbuf_h;

    // First and foremost, let's see if we can get SDL
    this->SDL_Status = this->init_SDL();
    
    // Now, let's check if SDL is up and running, and then init everything else.
    if (this->SDL_Status) {
        // Set the initial (background) drawing color.
        SDL_SetRenderDrawColor(
            this->renderer
            , this->background.r
            , this->background.g
            , this->background.b
            , 255
        );

        // Allocate pixel array
        this->init_pixel_array();

        //Present the renderer
        SDL_RenderPresent(this->renderer);

        // Init SDL input variables.
        this->state = SDL_GetKeyboardState(NULL);
        this->exit = false;
        
        SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(this->audioStream));

    }
    
    return;
}

chipperSDL3::~chipperSDL3() {
    // Clean up SDL Audio
    SDL_CloseAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK);
    // Clean up SDL Rendering/Textures
    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyTexture(this->render_texture);
    SDL_DestroyTexture(this->fade_texture);
    this->delete_pixel_array();
    this->renderer = NULL;
    this->render_texture = NULL;
    // Clean up Window
    SDL_DestroyWindow(this->window);
    this->window = NULL;
    // Exit SDL
    SDL_Quit();
    return;
}

void chipperSDL3::copy_screen(bool* data, int size) {
    // Build uint32_t array
    for (auto i = 0; i < size; i++) {
        this->pixel_array[i] = data[i] ? 0xFFFFFFFF : 0x000000FF;
    }
    return;
}

void chipperSDL3::refresh_screen() {
    
    // Set texture dimensions
    this->texrect.x = 0;
    this->texrect.y = 0;
    this->texrect.w = this->vbuf_w;
    this->texrect.h = this->vbuf_h;
    // Update texture with updated values
    // Normally you would want to stream the texture, but I found that there's
    // quite a lot more overhead with textures this small, than simply updating
    // it.   
    // TODO: Method to grab data from vram
    SDL_UpdateTexture(this->render_texture, NULL, this->pixel_array, 4 * this->vbuf_w);
    
    // Create rects for blit
    const SDL_FRect dstrect = {0, 0 , this->window_width, this->window_height};
    // Copy render_texture to fade_texture 
    SDL_SetRenderTarget(this->renderer, this->fade_texture);
    SDL_RenderTexture(this->renderer, this->render_texture, &this->texrect, &this->texrect);

    // Copy texture to the renderer and present
    SDL_SetRenderTarget(this->renderer, NULL);
    SDL_RenderTexture(this->renderer, this->fade_texture, &this->texrect, &dstrect);
    SDL_RenderPresent(this->renderer);
    return;
}

void chipperSDL3::set_resolution(int w, int h) {
    this->vbuf_w = w;
    this->vbuf_h = h;
    this->delete_textures();
    this->delete_pixel_array();
    this->init_textures();
    this->init_pixel_array();
    return;
}

int chipperSDL3::get_width() {
    return this->vbuf_w;
}

int chipperSDL3::get_height() {
    return this->vbuf_h;
}

// Implemented from tehBOOP
void chipperSDL3::process_keypad_events() {
    SDL_Event input;
    while (SDL_PollEvent(&input)) {
        switch (input.type) {
        case SDL_EVENT_QUIT:
            this->exit = true;
        break;
        case SDL_EVENT_WINDOW_RESIZED:
            // In this case, data1 is the width, and data2 is the height
            this->window_width = input.window.data1;
            this->window_height = input.window.data2;
            break;
        case SDL_EVENT_WINDOW_EXPOSED:
            // Repaint on exposure.
            SDL_RenderPresent(this->renderer);
            break;
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
            // The mouse has entered the window.
            this->is_mouse_focus = true;
            break;
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
            // The mouse has left the window.
            this->is_mouse_focus = false;
            break;
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
            // We have gained keyboard focus.
            this->is_keyboard_focus = true;
            break;
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            // We have lost the keyboard's focus.
            this->is_keyboard_focus = false;
            break;
        case SDL_EVENT_WINDOW_MINIMIZED:
            // The window has been minimized.
            this->is_minimized = true;
            break;
        case SDL_EVENT_WINDOW_MAXIMIZED:
            // The window has been maximized.
            this->is_minimized = false;
            break;
        case SDL_EVENT_WINDOW_RESTORED:
            // The window has been restored from a minimized state.
            this->is_minimized = false;
            break;
        default:
            // do_nothing();
            break;
        }

        // if (input.type == SDL_QUIT) {
        //     this->exit = true;
        // } // else do_nothing();
    }

    return;
}

bool chipperSDL3::get_exit_state() const {
    return this->exit;
}

bool chipperSDL3::is_key_pressed(unsigned char value) const {
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
unsigned char chipperSDL3::get_key_pressed() const {
    unsigned char key_pressed = 0x10;
    for (auto i = 0; i < 0x10; i++) {
        if (this->state[map[i].external_code] == 1) {
            key_pressed = i;
            i = 0x10;
        } // else, do_nothing();
    }
    return key_pressed;
}

// Implemented from tehBEEP

void chipperSDL3::copy_audio(uint8_t* data, int size) {
    SDL_PutAudioStreamData(this->audioStream, data, (Uint32)size);
    return;
}

int chipperSDL3::get_sample_rate() {
    return this->samplesPerSecond; 
}

int chipperSDL3::get_bytes_per_sample() {
    return this->bytesPerSample;
}

int chipperSDL3::get_buffer_size() {
    return SDL_GetAudioStreamQueued(this->audioStream);
}
