#include "tehAUDIO.h"

tehAUDIO::tehAUDIO(tehBEEP& in) {
    this->speaker = &in;

    this->toneHz = 200;
    this->toneVolume = 0xFFF;

    // Get the current sample-rate and sample size from the audio interface, and
    // configure buffer parameters accordingly
    this->samplesPerSecond = this->speaker->get_sample_rate();
    this->bytesPerSample = this->speaker->get_bytes_per_sample();
    this->squareWavePeriod = this->samplesPerSecond / this->toneHz;
    this->halfWavePeriod = this->squareWavePeriod / 2;
    this->sampleCount = this->samplesPerSecond / 60;
    this->bufferSize = (this->sampleCount * 4) * this->bytesPerSample;

    this->runningSampleIndex = 0;
    // Initialize, and allocate the RingBuffer.
    this->buffer.Size = this->bufferSize;
    this->buffer.writeCursor = 0;
    this->buffer.playCursor = 0;
    this->buffer.data = malloc(this->buffer.Size);

    // Zero out buffer
    for (int i = 0; i < this->buffer.Size ; i++) {
        // Cast the pointer to Uint8, offset by i, and dereference
        *((uint8_t*)this->buffer.data + i) = 0;
    }
}

tehAUDIO::~tehAUDIO() {
    free(this->buffer.data);
}

void tehAUDIO::sendDataToBuffer(int len) {
    // Initialize with Case B
    int Region1Size = len;
    int Region2Size = 0;
    // Test for Case A, overwrite if true
    // We've gotta do a bit more math here than when we're generating, so 
    // the logic's flipped around.
    // CASE B NEVER FIRES OFF- Are we doing something wrong?
    if (this->buffer.playCursor + len > this->buffer.Size) {
        Region1Size = this->buffer.Size - this->buffer.playCursor;
        Region2Size = len - Region1Size;
    }

    // Copy!
    this->speaker->copy_audio((uint8_t*)(this->buffer.data) + this->buffer.playCursor, Region1Size);
    this->speaker->copy_audio((uint8_t*)(this->buffer.data), Region2Size);
    this->buffer.playCursor = (this->buffer.playCursor + len) % this->buffer.Size;
    return;
}

/**
 * This function touches the read and write cursors for the ring buffer. Use 
 *  SDL_LockAudioDevice() before and SDL_UnlockAudioDevice() after calling this
 *  function to avoid undefined behavior.
 * 
 * NOTICE:
 * 
 * Either the output of this function is incorrect, audio playback is too in 
 *   Windows, or WINE is exhibiting an unknown issue. The tone sounds higher 
 *   pitched than it should.
 */

void tehAUDIO::GenerateSamples(bool mute) {
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
        (int16_t*)((uint8_t*)(this->buffer.data) + this->buffer.writeCursor);

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


void tehAUDIO::SoundTick(bool mute) {
    this->GenerateSamples(mute);
    int length = this->buffer.Size - this->speaker->get_buffer_size();
    this->sendDataToBuffer(length);
    return;
}