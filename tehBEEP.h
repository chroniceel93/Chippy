#ifndef TEHBEEP_H_
#define TEHBEEP_H_

#include <cstdint>

class tehBEEP {
    public:
    virtual ~tehBEEP() {}
    // TODO: Generic Documentation on what this should do

/**
 * @brief If necessary, fill the sound buffer.
 * 
 * This function, when implemented, should fill an audio buffer with either a 
 * square tone, or silence.
 * 
 * @param bool Mute on true, beep on false.
 */
    // virtual void SoundTick(bool mute) = 0;

    virtual void copy_audio(uint8_t* data, int size) = 0;

/**
 * @brief Returns the configured sample rate.
 */
    virtual int get_sample_rate() = 0;

/**
 * @brief Returns the size in bytes of each audio sample.
 */
    virtual int get_bytes_per_sample() = 0;

    virtual int get_buffer_size() = 0;
};

#endif
