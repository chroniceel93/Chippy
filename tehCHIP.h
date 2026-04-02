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

/**
 * @brief The entry point for the CHIP-8 Interpreter.
 * 
 * This class initializes all other classes, and provides the basic loop that
 *  drives all of the emulated components.
 */
class tehCHIP {
private:
    /** A pointer to the current ROM file. */
    tehROM *disk;
    /** A pointer to the current system BUS. */
    tehBUS *bus;
    /** A pointer to our processor. */
    tehCPUS *processor;
    /** Contains our current quirks mode. */
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

    /**
     * @brief Destructs the Chip-8 interpreter.
     */
    ~tehCHIP();

    /**
     * @brief Loads a ROM file to the system memory.
     * 
     * @param filename A  valid path to the ROM file we will load.
     */
    void load_program(std::string filename); 

    /**
     * @brief This function starts the main execution loop of the program.
     */
    void execute();

    /**
     * @brief This function resets the system.
     * 
     * TODO: Either re-create all objects, or make methods to reset their states
     */
    void reset_system();
    };
}

#endif