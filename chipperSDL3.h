/**
 * @file chippySDL.h
 * @author William Tradewell
 * @brief SDL3 Implementation of Interfaces.
 * @version 0.3
 * @date 2026-03-26
 */

#ifndef CHIPPERSDL3_H_
#define CHIPPERSDL3_H_

#include "tehSCREEN.h"
#include "tehBOOP.h"
#include "tehBEEP.h"
#include "tehCOMMONZ.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include "SDL3/SDL_audio.h"

/**
 * @brief chipperSDL3 implements the tehSCREEN, tehBEEP, and tehBOOP interfaces.
 * 
 * This class provides implementations for the tehSCREEN, tehBEEP, and tehBooP
 *  interfaces using SDL3 to handle screen drawing, video output, and input 
 *  handling. A pointer to this class can be passed to one or more objects
 *  expecting any of these these interfaces.
 * 
 * In fact, it would likely lead to problems if more than one instance of 
 *  chipperSDL3 were to be created. Please, instance chipperSDL3 once, and pass
 *  a pointer to that original object as needed.
 */
class chipperSDL3: public tehSCREEN, public tehBEEP, public tehBOOP {
private:
    struct mapping {
        SDL_Scancode external_code;
    };

    mapping map[16] {
        SDL_SCANCODE_X, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, 
        SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_A, 
        SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_Z, SDL_SCANCODE_C, 
        SDL_SCANCODE_4, SDL_SCANCODE_R, SDL_SCANCODE_F, SDL_SCANCODE_V
    };

    bool SDL_Status; // Hold copy of SDL Status code.

    // Variables used for framebuffer
    uint32_t *pixel_array; // Array of pixels used to build display texture.
    SDL_Texture *render_texture; // Pointer to the render texture.
    SDL_Texture *fade_texture;
    SDL_Renderer *renderer; // Pointer to the renderer.
    SDL_Color background, foreground; // TODO: use these to replace hardcoded values
    SDL_FRect texrect;
    int vbuf_h, vbuf_w;

    // Variables used to handle widnow state
    SDL_Window *window; // Pointer to our window.
    bool is_mouse_focus;
    bool is_keyboard_focus;
    bool is_fullscreen;
    bool is_minimized;

    // Window dimensions
    float window_height;
    float window_width;

    // Variables used for tehBOOP
    SDL_Event input; // input queue
    const bool *state;
    bool exit;

    // CONSTANT BLOCK
    // These vars define our audio output.
    const int samplesPerSecond = 48000;

    // for 16 bit, stereo audio, that's 4 bytes per sample
    const int bytesPerSample = sizeof(int16_t) * 2; 

    // These vars define our buffers.
    const int sampleCount = samplesPerSecond / 60;
    const int bufferSize = (sampleCount * 4) * bytesPerSample;

    // A struct holding all of the various audio configuration variables.
    SDL_AudioSpec audioSettings;
    SDL_AudioStream *audioStream;
 
    // Keeps track of what audio device we're using.
    int deviceID;
    
    // Keep this one around always increasing (and looping), so we have
    //   a constant tone.
    unsigned int runningSampleIndex;

    // Private initialization functions.
    bool init_SDL();
    bool init_SDL_Audio();
    bool init_SDL_window();
    bool init_renderer();
    bool init_textures();
    void init_pixel_array();

    // Helper functions to clean up allocated memory.
    void delete_textures();
    void delete_pixel_array();

public:
    chipperSDL3();
    ~chipperSDL3();

    // Implemented from tehSCREEN
    void copy_screen(bool* data, int size);
    void refresh_screen();
    void set_resolution(int w, int h);
    int get_width();
    int get_height();

    // Implemented from tehBOOP
    virtual void process_keypad_events();
    virtual bool get_exit_state() const;
    virtual bool is_key_pressed(unsigned char value) const;
    virtual unsigned char get_key_pressed() const;

    // Implemented from tehBEEP
    void copy_audio(uint8_t* data, int size);
    int get_sample_rate();
    int get_bytes_per_sample();
    int get_buffer_size();
};

#endif