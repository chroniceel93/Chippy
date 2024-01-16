#include "tehBEEP.h"


tehBEEP::tehBEEP() {
    SDL_InitSubSystem(SDL_INIT_AUDIO);

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
    // Once SDL's opened the audio device, it replaces these values with what it
    //  is actually using.
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
    
    // Some of the math we're doing here *counts* on us using the S16LSB format.
    // It'll still run if we don't have that, but it might sound pretty wild.
    // Print out an error in this case, so we know why your speakers are 
    //  cracklin'.
    if (this->audioSettings.format != AUDIO_S16LSB) {
        SDL_LogMessage(0, SDL_LOG_PRIORITY_CRITICAL, 
            "Unexpected Audio Format.");
    }

    // SDL initially stops audio playback. This resumes it.
    SDL_PauseAudioDevice(this->deviceID, 0);
    return;
}

tehBEEP::~tehBEEP() {
    // Cleaning up after ourselves. Don't litter!
    free(this->buffer.data);
    SDL_PauseAudioDevice(this->deviceID, 1);
    return;
}

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
 * @param bool Mute on true, beep on false.
 */
void tehBEEP::GenerateSamples(bool mute) {
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
void tehBEEP::SoundTick(bool mute) {
    SDL_LockAudioDevice(this->deviceID);
    this->GenerateSamples(mute);
    SDL_UnlockAudioDevice(this->deviceID);
    return;
}
