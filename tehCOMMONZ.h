#ifndef TEHCOMMONZ_H_
#define TEHCOMMONZ_H_

/**
 * @brief Tests to see if the current quirk mode is in the CHIP48 family.
 */
#define IS_HP48(a) \
    ((a == chippy::CHIP48) \
    || (a == chippy::SUPERCHIP10) \
    || (a == chippy::SUPERCHIP11) ? true : false)

/**
 * @brief Tests to see if the current quirk mode is in the SUPERCHIP family.
 */
#define IS_SUPERCHIP(a) \
    ((a == chippy::SUPERCHIP10) \
    || (a == chippy::SUPERCHIP11)  ? true : false)

const int WINDOW_HEIGHT = 256;
const int WINDOW_WIDTH = 512;

namespace chippy{ 
    enum systype {
        CHIP8, CHIP48, SUPERCHIP10, SUPERCHIP11
    };    
}

#endif