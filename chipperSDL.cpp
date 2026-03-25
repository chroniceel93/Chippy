#include "chipperSDL.h"

// Init SDL
bool chipperSDL::init_SDL() {
    bool result = true;
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        throw SDL_GetError();
        result = false;
    } else {
        if (this->init_SDL_window()) {
            if (this->init_renderer()) {
                if (!this->init_textures()) result = false;
            } else {
                result = false;
            }

        } else {
            result = false;
        }
    }
    return result;
}

bool chipperSDL::init_SDL_window() {
    bool result = true;
    // create window
    this->window = SDL_CreateWindow(
        "Chip-8"
        , SDL_WINDOWPOS_CENTERED
        , SDL_WINDOWPOS_CENTERED
        , WINDOW_WIDTH
        , WINDOW_HEIGHT
        , SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    // if the SDL_CreateWindow fails, then the pointer *window will be null. 
    // So, if the window == nullptr, then throw an error
    if (this->window == NULL) {
        throw SDL_GetError();
        result = false;
    } else {
        this->is_mouse_focus = true;
        this->is_keyboard_focus = true;
        this->window_width = WINDOW_WIDTH;
        this->window_height = WINDOW_HEIGHT;
    }
    return result;
}

bool chipperSDL::init_renderer() {
    bool result = true;
    // Attempt to create a renderer
    // Keep flags at 0 - SDL selects hardware renderers first, and falls back to
    // software if not available.
    this->renderer = SDL_CreateRenderer(
        this->window
        , -1
        , 0 
    );

    if (this->renderer == NULL) {
        throw SDL_GetError();
        result = false;
    } 

    return result;
}

bool chipperSDL::init_textures() {
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
    }

    return result;
}

void chipperSDL::delete_textures() {
    SDL_DestroyTexture(this->render_texture);
    SDL_DestroyTexture(this->fade_texture);
    this->render_texture = NULL;
    this->fade_texture = NULL;
    return;
}

void chipperSDL::init_pixel_array() {
    int array_size = sizeof(uint32_t) * this->vbuf_h * this->vbuf_w;
    this->pixel_array = (uint32_t*) malloc(array_size);
    return;
}

void chipperSDL::delete_pixel_array() {
    free(this->pixel_array);
    this->pixel_array = NULL;
    return;
}


chipperSDL::chipperSDL() {
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

        // Initialize class variables

        // We fill audioSettings with the values we want, and pass that to SDL.
        // Once SDL's opened the audio device, it replaces these values with
        // what it is actually using.
        this->audioSettings.freq = this->samplesPerSecond;
        this->audioSettings.format = AUDIO_S16LSB;
        this->audioSettings.channels = 2;
        this->audioSettings.samples = this->sampleCount;
        this->audioSettings.callback = NULL;
        // this->audioSettings.userdata = (void*)&this->buffer;
        this->deviceID = SDL_OpenAudioDevice(NULL // Device: Null = default
                            , 0 // iscapture: 0 = output device
                            , &this->audioSettings // Pointer to audioSettings
                            , 0 // Used to return audioSettings struct.
                            , SDL_AUDIO_ALLOW_ANY_CHANGE);

        // Some of the math we're doing here *counts* on us using the S16LSB 
        // format. It'll still run if we don't have that, but it might sound 
        // pretty wild. Print out an error in this case, so we know why your 
        // speakers are cracklin'.
        if (this->audioSettings.format != AUDIO_S16LSB) {
            SDL_LogMessage(0, SDL_LOG_PRIORITY_CRITICAL, 
                "Unexpected Audio Format.");
        }

        // SDL initially stops audio playback. This resumes it.
        SDL_PauseAudioDevice(this->deviceID, 0);
    }
    
    return;
}


chipperSDL::~chipperSDL() {
    // Clean up SDL Audio
    SDL_PauseAudioDevice(this->deviceID, 1);
    SDL_CloseAudioDevice(this->deviceID);
    // Clean up SDL Rendering
    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyWindow(this->window);
    SDL_DestroyTexture(this->render_texture);
    SDL_DestroyTexture(this->fade_texture);
    // this->delete_pixel_array();
    this->renderer = NULL;
    this->window = NULL;
    this->render_texture = NULL;
    SDL_Quit();
    return;
}

void chipperSDL::copy_screen(bool* data, int size) {
    // Build uint32_t array
    for (auto i = 0; i < size; i++) {
        this->pixel_array[i] = data[i] ? 0xFFFFFFFF : 0x000000FF;
    }
    return;
}

void chipperSDL::refresh_screen() {
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
    SDL_Rect dstrect;
    dstrect.x = 0;
    dstrect.y = 0;
    dstrect.w = this->window_width;
    dstrect.h = this->window_height;
    // Copy render_texture to fade_texture 
    SDL_SetRenderTarget(this->renderer, this->fade_texture);
    SDL_RenderCopy(this->renderer, this->render_texture, &this->texrect, &this->texrect);

    // Copy texture to the renderer and present
    SDL_SetRenderTarget(this->renderer, NULL);
    SDL_RenderCopy(this->renderer, this->fade_texture, &this->texrect, &dstrect);
    SDL_RenderPresent(this->renderer);
    return;
}

void chipperSDL::set_resolution(int w, int h) {
    this->vbuf_w = w;
    this->vbuf_h = h;
    this->delete_textures();
    this->delete_pixel_array();
    this->init_textures();
    this->init_pixel_array();
    return;
}

int chipperSDL::get_width() {
    return this->vbuf_w;
}

int chipperSDL::get_height() {
    return this->vbuf_h;
}

// Implemented from tehBOOP
void chipperSDL::process_events() {
    SDL_Event input;
    while (SDL_PollEvent(&input)) {
        switch (input.type) {
        case SDL_QUIT:
            this->exit = true;
            break;
        case SDL_WINDOWEVENT:
            
            switch (input.window.event) {
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                // In this case, data1 is the width, and data2 is the height
                this->window_width = input.window.data1;
                this->window_height = input.window.data2;
                break;
            case SDL_WINDOWEVENT_EXPOSED:
                // Repaint on exposure.
                SDL_RenderPresent(this->renderer);
                break;
            case SDL_WINDOWEVENT_ENTER:
                // The mouse has entered the window.
                this->is_mouse_focus = true;
                break;
            case SDL_WINDOWEVENT_LEAVE:
                // The mouse has left the window.
                this->is_mouse_focus = false;
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                // We have gained keyboard focus.
                this->is_keyboard_focus = true;
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                // We have lost the keyboard's focus.
                this->is_keyboard_focus = false;
                break;
            case SDL_WINDOWEVENT_MINIMIZED:
                // The window has been minimized.
                this->is_minimized = true;
                break;
            case SDL_WINDOWEVENT_MAXIMIZED:
                // The window has been maximized.
                this->is_minimized = false;
                break;
            case SDL_WINDOWEVENT_RESTORED:
                // The window has been restored from a minimized state.
                this->is_minimized = false;
                break;
            default:
                // do_nothing();
                break;
            }

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

// Implemented from tehBEEP

void chipperSDL::copy_audio(uint8_t* data, int size) {
    SDL_QueueAudio(this->deviceID, data, (Uint32)size);
    return;
}

int chipperSDL::get_sample_rate() {
    return this->samplesPerSecond; 
}

int chipperSDL::get_bytes_per_sample() {
    return this->bytesPerSample;
}

int chipperSDL::get_buffer_size() {
    return SDL_GetQueuedAudioSize(this->deviceID);
}