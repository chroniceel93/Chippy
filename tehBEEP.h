/**
 * @file tehBEEP.h
 * @author William Tradewell
 * @brief A virtual interface for handling audio output.
 * @version 0.2
 * @date 2026-03-27
 */

#ifndef TEHBEEP_H_
#define TEHBEEP_H_

#include <cstdint>

/**
 * @brief tehBOOP is a virtual interface for handling audio output.
 * 
 * This class is a virtual interface that defines a minimum viable set of
 *  functions needed to send audio to an output device.
 * 
 * This interface assumes an audio API that is capable of copying given data
 *  into a buffer. 
 */
class tehBEEP {
    public:
    virtual ~tehBEEP() {}
    
/**
 * @brief Copy the given data to the sound buffer.
 * 
 * Given a pointer to a data buffer, and the size of that buffer, copy the data
 *  to the implementation's sound buffer.
 * 
 * @param data Pointer to the sound data to be copied.
 * @param size Number of bytes to copy to the sound buffer.
 */
    virtual void copy_audio(uint8_t* data, int size) = 0;

/**
 * @brief Returns the configured sample rate.
 * 
 * @return int - The configured sample rate.
 */
    virtual int get_sample_rate() = 0;

/**
 * @brief Returns the size in bytes of any audio sample.
 * 
 * @return int - The size in bytes of any audio sample.
 */
    virtual int get_bytes_per_sample() = 0;

/**
 * @brief Returns the size in bytes of queued audio in the buffer.
 * 
 * @return int - The size in bytes of queued audio in the buffer.
 */
    virtual int get_buffer_size() = 0;
};

#endif
