/**
 * @file tehCHIP.h
 * @author William Tradewell
 * @brief This class manages overall emulation tasks.
 * @version 0.1
 * @date 2026-03-30
 */

#ifndef TEHCHIP_H_
#define TEHCHIP_H_

#include <chrono>
#include <thread>

#include "tehCOMMONZ.h"

#include "tehROM.h"
#include "tehBUS.h"
#include "tehCPUS.h"

namespace chippy {

    class tehCHIP {
private:
    tehROM *disk;
    tehBUS *bus;
    tehCPUS *processor;
    systype operating_mode;

public:
    /**
     * @brief Initializes the Chip-8 interpreter.
     * 
     * @param s Reference to a screen object, to be used by the system.
     * @param b Reference to a speaker object, to be used by the system.
     * @param k Reference to an input object, to be used by the system.
     * @param opMode Sets the quirks and operating mode of the system.
     */
    tehCHIP(tehSCREEN& s, tehBEEP& b, tehBOOP& k, tehGUI& g, systype opMode);

    ~tehCHIP();

    void load_program(std::string filename); 

    void execute();

    void reset_system();
    };
}

#endif