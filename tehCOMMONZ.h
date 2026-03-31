/**
 * @file tehCOMMONZ.h
 * @author William Tradewell
 * @brief Here we define useful macros, constants and structs for emulation.
 * @version 0.1
 * @date 2026-03-30
 */

#ifndef TEHCOMMONZ_H_
#define TEHCOMMONZ_H_

/**
 * @brief Tests to see if the current quirk mode is in the CHIP48 family.
 * 
 * This currently includes HP48, SUPERCHIP10, SUPERCHIP11
 * 
 * @param a The current quirks mode as a systype enum.
 * @return True if current quirk is part of the HP48 family.
 */
#define IS_HP48(a) \
    ((a == chippy::CHIP48) \
    || (a == chippy::SUPERCHIP10) \
    || (a == chippy::SUPERCHIP11) ? true : false)

/**
 * @brief Tests to see if the current quirk mode is in the SUPERCHIP family.
 * 
 * This currently includes SUPERCHIP10, SUPERCHIP11
 * 
 * @param a The current quirks mode as a systype enum.
 * @return True if quirk is part of the SUPERCHIP family.
 */
#define IS_SUPERCHIP(a) \
    ((a == chippy::SUPERCHIP10) \
    || (a == chippy::SUPERCHIP11)  ? true : false)

const int DEFAULT_WINDOW_HEIGHT = 256;
const int DEFAULT_WINDOW_WIDTH = 512;

namespace chippy{ 
    enum systype {
        CHIP8, CHIP48, SUPERCHIP10, SUPERCHIP11
    };    
}

#endif