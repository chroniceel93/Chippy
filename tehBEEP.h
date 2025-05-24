#ifndef TEHBEEP_H_
#define TEHBEEP_H_

class tehBEEP {
    public:
    virtual ~tehBEEP() {}
    // TODO: Generic Documentation on what this should do
    virtual void SoundTick(bool mute) = 0;
};

#endif
