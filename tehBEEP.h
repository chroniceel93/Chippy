#ifndef TEHBEEP_H_
#define TEHBEEP_H_

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
    virtual void SoundTick(bool mute) = 0;
};

#endif
