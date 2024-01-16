#ifndef TEHBEEP_H_
#define TEHBEEP_H_

#include "SDL2/SDL.h"
#include "SDL2/SDL_audio.h"

class tehBEEP {
private:
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

    static void SDLAudioCallback(void *UserData, Uint8 *AudioData, int Length) {
        
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

    void GenerateSamples(bool mute);

public:

    tehBEEP();
    ~tehBEEP();
    void SoundTick(bool mute);

};

#endif
