/**
 * @file tehAUDIO.h
 * @author William Tradewell
 * @brief This class provides our CHIP-8 beeper emulation.
 * @version 0.1
 * @date 2026-03-30
 */

#ifndef TEH_AUDIO_H_
#define TEH_AUDIO_H_

#include "tehBEEP.h"

#include <cstdint>
#include <cstdlib>

class tehAUDIO {
private:
    tehBEEP *speaker;

    // Variables used for our ring buffer
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

    audio_ring_buffer buffer;
        // CONSTANT BLOCK
    // These vars define our audio output.
    int samplesPerSecond;
    // for 16 bit, stereo audio, that's 4 bytes per sample
    int bytesPerSample; 
    // These vars define our tone.
    int toneHz;
    short int toneVolume;
    int squareWavePeriod;
    int halfWavePeriod;
    // These vars define our buffers.
    int sampleCount;
    int bufferSize;

    // Keep this one around always increasing (and looping), so we have
    //   a constant tone.
    unsigned int runningSampleIndex;

    void sendDataToBuffer(int len);
public:
    
    tehAUDIO(tehBEEP& in);

    ~tehAUDIO();

    /**
     * @brief Fill buffer with audio.
     * 
     * Generates silence when mute is true, and a square wave when mute is false. 
     * 
     * Writes sound data into a ring buffer.
     * 
     * @param bool Mute on true, beep on false.
     */
    void GenerateSamples(bool mute);

    /**
     * @brief If necessary, fill the sound buffer.
     * 
     * This function, when implemented, should fill an audio buffer with either a 
     * square tone, or silence.
     * 
     * @param bool Mute on true, beep on false.
     */
    void SoundTick(bool mute);

};

#endif