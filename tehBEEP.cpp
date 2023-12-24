#include "tehBEEP.h"


tehBEEP::tehBEEP() {
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    this->runningSampleIndex = 0;

    this->audioSettings = {0};
    this->audioSettings.freq = this->samplesPerSecond;
    this->audioSettings.format = AUDIO_S16LSB;
    this->audioSettings.channels = 2;
    this->audioSettings.samples = this->sampleCount;

    SDL_OpenAudio(&this->audioSettings, 0);

    if (this->audioSettings.format != AUDIO_S16LSB) {
        // TODO: Throw error, audio type wrong.
        SDL_LogMessage(0, SDL_LOG_PRIORITY_CRITICAL, "beep");
    }
    this->GenerateSamples(true);
    SDL_PauseAudio(0);
}

void tehBEEP::GenerateSamples(bool mute) {
    int16_t sampleValue = 0;
    // Re-set pointer to beginning of buffer
    this->soundBuffer = malloc(this->bytesPerSample * this->sampleCount);
    this->sampleOut = (int16_t *) this->soundBuffer;
    for (int sampleIndex = 0; sampleIndex < this->sampleCount; sampleIndex++) {
        sampleValue = 
            ((this->runningSampleIndex++ / this->halfWavePeriod) % 2)
            ? this->toneVolume
            : -this->toneVolume;
        sampleValue = (!mute) ? sampleValue : 0;
        *this->sampleOut++ = sampleValue;
        *this->sampleOut++ = sampleValue;
    }
    return;
}

void tehBEEP::MakeSomeNoize() {
    SDL_QueueAudio(1, this->soundBuffer, this->bytesPerSample * this->sampleCount);
    free(this->soundBuffer);
    return;
}

void tehBEEP::SoundTick(bool mute) {
    this->GenerateSamples(mute);
    this->MakeSomeNoize();
    return;
}