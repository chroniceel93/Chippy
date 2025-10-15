/**
 * @file chippySDL.h
 * @author William Tradewell
 * @brief SDL2 Implementation of Interfaces.
 * @version 0.2
 * @date 2025-05-23
 */

#include "tehSCREEN.h"
#include "tehBOOP.h"
#include "tehBEEP.h"
#include "SDL.h"
#include "SDL_audio.h"

#ifndef CHIPPERSDL_H_
#define CHIPPERSDL_H_

class chipperSDL: public tehSCREEN, public tehBOOP, public tehBEEP {
private:
    // Variables used for tehSCREEN
    bool SDL_Status; // Hold copy of SDL Status code.
    uint32_t pixels[2048]; // Array of pixels used to build display texture.
    SDL_Window *window; // Pointer to our window.
    SDL_Texture *render_texture; // Pointer to the render texture.
    SDL_Texture *fade_texture;
    SDL_Renderer *renderer; // Pointer to the renderer.
    SDL_Color background, foreground; // TODO: use these to replace hardcoded values
    SDL_Rect texrect;
    int vbuf_h, vbuf_w;

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

    // Variables used for tehBEEP
        // Credit to David Gow's Handmade Penguin tutorial, Getting Circular with
    //  SDL Audio by Eric Scrivner, and to LazyFoo's SDL tutorial, with which I 
    //  kludged together something that barely works!
    // https://lazyfoo.net/tutorials/SDL/
    // https://davidgow.net/handmadepenguin/ch8.html
    // https://ericscrivner.me/2017/10/getting-circular-sdl-audio/

    struct audio_ring_buffer {
        int Size; // Size of our buffer, in bytes
        int writeCursor; // Where we will insert data into our buffer
        int playCursor; // Where we will read data from the buffer
        void *data;
    };

    // CONSTANT BLOCK
    // These vars define our audio output.
    const int samplesPerSecond = 48000;
    // for 16 bit, stereo audio, that's 4 bytes per sample
    const int bytesPerSample = sizeof(int16_t) * 2; 
    // These vars define our tone.
    const int toneHz = 200;
    const int16_t toneVolume = 0xFFF;
    const int squareWavePeriod = samplesPerSecond / toneHz;
    const int halfWavePeriod = squareWavePeriod / 2;
    // These vars define our buffers.
    const int sampleCount = samplesPerSecond / 60;
    const int bufferSize = (sampleCount * 4) * bytesPerSample;
    
    audio_ring_buffer buffer;

    // A struct holding all of the various audio configuration variables.
    SDL_AudioSpec audioSettings;
 
    // Keeps track of what audio device we're using.
    int deviceID;
    
    // Keep this one around always increasing (and looping), so we have
    //   a constant tone.
    unsigned int runningSampleIndex;

    // Private initialization functions
    bool init_SDL();
    bool init_SDL_window();
    bool init_renderer();
    bool init_textures();

    void delete_textures();
    
    // Private functions for audio handling.
    static void SDLAudioCallback(void *UserData, Uint8 *AudioData, int Length);
    void GenerateSamples(bool mute);

public:
    chipperSDL();
    ~chipperSDL();

    // Implemented from tehSCREEN
    void blank_screen();
    bool draw_point(int x, int y);
    void refresh_screen();
    void set_resolution(int h, int w);

    // Implemented from tehBOOP
    virtual void process_events();
    virtual bool get_exit_state() const;
    virtual bool is_key_pressed(unsigned char value) const;
    virtual unsigned char get_key_pressed() const;

    // Implemented from tehBEEP
    void SoundTick(bool mute);
};

#endif