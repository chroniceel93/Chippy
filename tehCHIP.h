#ifndef TEHCHIP_H_
#define TEHCHIP_H_

#include <chrono>
#include <thread>

#include "tehROM.h"
#include "tehBUS.h"
#include "tehCPUS.h"

class tehCHIP {
private:
    tehROM *disk;
    tehBUS *bus;
    tehCPUS *processor;
public:
    enum systype {
        CHIP8, SUPERCHIP
    };

    tehCHIP(tehSCREEN& s, tehBEEP& b, tehBOOP& k);

    // ~tehCHIP();

    void load_program(std::string filename); 

    void execute();

    void reset_system();
};

#endif