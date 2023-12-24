#ifndef TEHBEEP_H_
#define TEHBEEP_H_

#include "SDL2/SDL.h"
#include "SDL2/SDL_audio.h"

class tehBEEP {
private:
    // Credit to David Gow's Handmade Penguin tutorial and to LazyFoo's SDL
    // tutorial, with which I kludged together something that barely works!
    const int samplesPerSecond = 48000;
    const int toneHz = 300;
    const int16_t toneVolume = 3000;
    const int squareWavePeriod = samplesPerSecond / toneHz;
    const int halfWavePeriod = squareWavePeriod / 2;
    const int sampleCount = 800;
    const int bytesPerSample = sizeof(int16_t) * 2;
    const int bytesToWrite = sampleCount * bytesPerSample;

    unsigned int runningSampleIndex;

    void *soundBuffer; // this is where things get *nasty*.
    int16_t *sampleOut;

    SDL_AudioSpec audioSettings;

    void GenerateSamples(bool mute);

    void MakeSomeNoize();
public:

    tehBEEP();
    void SoundTick(bool mute);

};

#endif