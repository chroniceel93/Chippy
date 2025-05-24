#ifndef TEHBOOP_H_
#define TEHBOOP_H_

class tehBOOP {
public:
    // virtual ~tehBOOP() {}
    virtual void process_events() = 0;
    virtual bool get_exit_state() const = 0;
    virtual bool is_key_pressed(unsigned char value) const = 0;
    virtual unsigned char get_key_pressed() const = 0;
};

#endif