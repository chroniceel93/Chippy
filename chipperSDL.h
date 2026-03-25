/**
 * @file chippySDL.h
 * @author William Tradewell
 * @brief SDL2 Implementation of Interfaces.
 * @version 0.2
 * @date 2025-05-23
 */

#ifndef CHIPPERSDL_H_
#define CHIPPERSDL_H_

#include "tehSCREEN.h"
#include "tehBOOP.h"
#include "tehBEEP.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_audio.h"

#include <iostream>

const int WINDOW_HEIGHT = 256;
const int WINDOW_WIDTH = 512;

class chipperSDL: public tehSCREEN, public tehBOOP, public tehBEEP {
private:
    // Variables used for tehSCREEN
    bool SDL_Status; // Hold copy of SDL Status code.
    uint32_t *pixel_array; // Array of pixels used to build display texture.
    SDL_Texture *render_texture; // Pointer to the render texture.
    SDL_Texture *fade_texture;
    SDL_Renderer *renderer; // Pointer to the renderer.
    SDL_Color background, foreground; // TODO: use these to replace hardcoded values
    SDL_Rect texrect;
    int vbuf_h, vbuf_w;
    // Variables used to handle widnow state
    SDL_Window *window; // Pointer to our window.
    bool is_mouse_focus;
    bool is_keyboard_focus;
    bool is_fullscreen;
    bool is_minimized;
    // Window dimensions
    int window_height;
    int window_width;

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

    // A struct holding all of the various audio configuration variables.
    SDL_AudioSpec audioSettings;
    SDL_AudioStream *stream;
    // Keeps track of what audio device we're using.
    int deviceID;
    // CONSTANT BLOCK
    // These vars define our audio output.
    const int samplesPerSecond = 48000;
    // for 16 bit, stereo audio, that's 4 bytes per sample
    const int bytesPerSample = sizeof(int16_t) * 2; 
    // These vars define our tone.
    // const int toneHz = 200;
    // const short int toneVolume = 0xFFF;
    // const int squareWavePeriod = samplesPerSecond / toneHz;
    // const int halfWavePeriod = squareWavePeriod / 2;
    // These vars define our buffers.
    const int sampleCount = samplesPerSecond / 60;
    // const int bufferSize = (sampleCount * 4) * bytesPerSample;
    int16_t bufferPointer;
    int bufferSize;
    
    // Private initialization functions
    bool init_SDL();
    bool init_SDL_window();
    bool init_renderer();
    bool init_textures();
    void init_pixel_array();

    void delete_textures();
    void delete_pixel_array();
    
    // Private functions for audio handling.
    // static void SDLAudioCallback(void *UserData, Uint8 *AudioData, int Length);


public:
    chipperSDL();
    ~chipperSDL();

    // Implemented from tehSCREEN
    // void blank_screen();
    // bool draw_point(int x, int y);
    void copy_screen(bool* data, int size);
    void refresh_screen();
    void set_resolution(int w, int h);
    int get_width();
    int get_height();


    // Implemented from tehBOOP
    virtual void process_events();
    virtual bool get_exit_state() const;
    virtual bool is_key_pressed(unsigned char value) const;
    virtual unsigned char get_key_pressed() const;

    // Implemented from tehBEEP
    // void SoundTick(bool mute);

    void copy_audio(uint8_t* data, int size);

    int get_sample_rate();

    int get_bytes_per_sample();

    int get_buffer_size();
};

#endif