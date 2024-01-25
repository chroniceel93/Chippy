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
    tehBUS bus = tehBUS();
    tehCPUS processor = tehCPUS(this->bus);
public:

    tehCHIP();

    // ~tehCHIP();

    void load_program(std::string filename); 

    void execute();

    void reset_system();
};

#endif