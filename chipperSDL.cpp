#include "chipperSDL.h"

void chipperSDL::SDLAudioCallback(void *UserData, Uint8 *AudioData, int Length) {
    audio_ring_buffer *RingBuffer = (audio_ring_buffer *) UserData;

    // Initialize with Case B
    int Region1Size = Length;
    int Region2Size = 0;
    // Test for Case A, overwrite if true
    // We've gotta do a bit more math here than when we're generating, so 
    // the logic's flipped around.
    // CASE B NEVER FIRES OFF- Are we doing something wrong?
    if (RingBuffer->playCursor + Length > RingBuffer->Size) {
        Region1Size = RingBuffer->Size - RingBuffer->playCursor;
        Region2Size = Length - Region1Size;
    }

    // Copy!
    memcpy(AudioData, (Uint8*)(RingBuffer->data) + RingBuffer->playCursor, Region1Size);
    memcpy(&AudioData[Region1Size], RingBuffer->data, Region2Size);
    RingBuffer->playCursor = (RingBuffer->playCursor + Length) % RingBuffer->Size;
    return;
}

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
        , 512
        , 256
        , SDL_WINDOW_SHOWN
    );
    // if the SDL_CreateWindow fails, then the pointer *window will be null. 
    // So, if the window == nullptr, then throw an error
    if (this->window == NULL) {
        throw SDL_GetError();
        result = false;
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
        , 64
        , 32
    );

    SDL_SetTextureBlendMode(this->render_texture, SDL_BLENDMODE_BLEND);

    this->fade_texture = SDL_CreateTexture(
        this->renderer
        , SDL_PIXELFORMAT_RGBA8888
        , SDL_TEXTUREACCESS_TARGET
        , 64
        , 32
    );

    // If either texture is null, something is wrong. Throw an error and set
    // result to false.
    if (this->render_texture == NULL || this->fade_texture == NULL) {
        throw SDL_GetError();
        result = false;
    }

    return result;
}


chipperSDL::chipperSDL() {
    this->SDL_Status = true; // Assume SDL is good- Set to false if init fails
    this->background.r = 0;
    this->background.g = 0;
    this->background.b = 0;
    this->foreground.r = 255;
    this->foreground.g = 255;
    this->foreground.b = 255;

    // We're setting the default dimensions to match the Chip-8 video buffer.
    this->texrect.x = 0;
    this->texrect.y = 0;
    this->texrect.w = 64;
    this->texrect.h = 32;
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
        // blank the screen
        this->blank_screen();

        //Present the renderer
        SDL_RenderPresent(this->renderer);

        // Init SDL input variables.
        this->state = SDL_GetKeyboardState(NULL);
        this->exit = false;

        // // Init SDL Audio subsystem
        // SDL_InitSubSystem(SDL_INIT_AUDIO);

        // Initialize class variables
        this->runningSampleIndex = 0;
        // Initialize, and allocate the RingBuffer.
        this->buffer.Size = this->bufferSize;
        this->buffer.writeCursor = 0;
        this->buffer.playCursor = 0;
        this->buffer.data = malloc(this->buffer.Size);

        // Zero out buffer
        for (int i = 0; i < this->buffer.Size ; i++) {
            // Cast the pointer to Uint8, offset by i, and dereference
            *((Uint8*)this->buffer.data + i) = 0;
        }

        // We fill audioSettings with the values we want, and pass that to SDL.
        // Once SDL's opened the audio device, it replaces these values with
        // what it is actually using.
        this->audioSettings.freq = this->samplesPerSecond;
        this->audioSettings.format = AUDIO_S16LSB;
        this->audioSettings.channels = 2;
        this->audioSettings.samples = this->sampleCount;
        this->audioSettings.callback = this->SDLAudioCallback;
        this->audioSettings.userdata = (void*)&this->buffer;
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
    free(this->buffer.data);
    SDL_PauseAudioDevice(this->deviceID, 1);
    // Clean up SDL Rendering
    SDL_DestroyRenderer(this->renderer);
    SDL_DestroyWindow(this->window);
    SDL_DestroyTexture(this->render_texture);
    SDL_DestroyTexture(this->fade_texture);
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
    
    // Create rects for blit
    SDL_Rect dstrect;
    dstrect.x = 0;
    dstrect.y = 0;
    dstrect.w = 512;
    dstrect.h = 256;
    // Copy render_texture to fade_texture 
    SDL_SetRenderTarget(this->renderer, this->fade_texture);
    SDL_RenderCopy(this->renderer, this->render_texture, &this->texrect, &this->texrect);

    // Copy texture to the renderer and present
    SDL_SetRenderTarget(this->renderer, NULL);
    SDL_RenderCopy(this->renderer, this->fade_texture, &this->texrect, &dstrect);
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

// Implemented from tehBEEP

/**
 * @brief Fill buffer with audio.
 * 
 * Generates silence when mute is true, and a square wave when mute is false. 
 * 
 * Writes sound data into a ring buffer.
 * 
 * This function touches the read and write cursors for the ring buffer. Use 
 *  SDL_LockAudioDevice() before and SDL_UnlockAudioDevice() after calling this
 *  function to avoid undefined behavior.
 * 
 * NOTICE:
 * 
 * Either the output of this function is incorrect, audio playback is too in 
 *   Windows, or WINE is exhibiting an unknown issue. The tone sounds higher 
 *   pitched than it should.
 * 
 * @param bool Mute on true, beep on false.
 */
void chipperSDL::GenerateSamples(bool mute) {
    // Initialize with Case A - The Play Cursor is ahead of the Write Cursor
    // In case A, we generate samples up to the play cursor in one contiguous 
    //  block
    int W1 = this->buffer.playCursor - this->buffer.writeCursor;
    int W2 = 0;
    // Test for Case B - The Play cursor is behind Write Cursor
    if (this->buffer.writeCursor > this->buffer.playCursor) {
    // In case B, we generate samples up to the end of the buffer, and then 
    // generate samples from the beginning of the buffer, up to the play cursor.
        W1 = this->buffer.Size - this->buffer.writeCursor;
        W2 = this->buffer.playCursor;
    }

    int16_t sampleValue = 0; // Holds the evaluated sample value in our loops.

    // This one... Is fun.
    // Our Ring Buffer object holds the audio data as a void pointer to a pre-
    //  allocated block of memory. We use (Uint8*) to cast this void pointer to
    //  a 8-bit pointer. The specific type isn't important, as the width.
    // When we iterate that pointer, it will progress in 8 bit steps. This gets
    //  us a pointer to the right location.
    // Then, we cast the pointer to int16_t- The same type as our audio samples.
    //  This lets us iterate cleanly forwards along the buffer when generating
    //  audio.
    int16_t* dest = 
        (int16_t*)((Uint8*)(this->buffer.data) + this->buffer.writeCursor);

    // If mute is true, zero out tone, otherwise, set it to 
    int tone = (mute) ? 0 : this->toneVolume;

    // We have duplicate logic here! While we could split this part into its own
    //  function, this is already a time-critical task, and we don't need the 
    //  extra overhead.

    // The number of samples we need to generate can be calculated by dividing
    //  the size of the desired amount of buffer by the size, in bytes a single
    //  sample takes up.
    int samples = W1 / this->bytesPerSample;

    // Generating samples is dead simple, now that we know where to start, and
    //  how far to go. Each loop, we determine if the waveform is high, or low-
    //  And then generate the sample twice- Necessary for stereo audio.
    for (int i = 0; i < samples; i++) {
        sampleValue = 
            ((this->runningSampleIndex++ / this->halfWavePeriod) % 2)
                ? tone : -tone;
        *dest++ = sampleValue;
        *dest++ = sampleValue;
    }

    // For case B set dest to the beginning of the data buffer.
    dest = (int16_t*)(this->buffer.data);

    // Repeat for section two. If section two is empty, no logic will be execu-
    //  ted here.
    samples = W2 / this->bytesPerSample;

    for (int i = 0; i < samples; i++) {
        sampleValue =
            ((this->runningSampleIndex++ / this->halfWavePeriod) % 2)
                ? tone : - tone;
        *dest++ = sampleValue;
        *dest++ = sampleValue;
    }

    // After everything is over, we should be all caught up!
    this->buffer.writeCursor = this->buffer.playCursor;
    return;
}

/**
 * @brief If necessary, fill the sound buffer.
 * 
 * @param bool Mute on true, beep on false.
*/
void chipperSDL::SoundTick(bool mute) {
    SDL_LockAudioDevice(this->deviceID);
    this->GenerateSamples(mute);
    SDL_UnlockAudioDevice(this->deviceID);
    return;
}
