/**
 * @file tehBUS.h
 * @author William Tradewell
 * @brief A class representing a system bus.
 * @version 0.1
 * @date 2026-03-30
 */

#ifndef TEHBUS_H_
#define TEHBUS_H_

#include "tehCOMMONZ.h"
#include "tehRAMS.h"
#include "tehVIDEO.h"
#include "tehAUDIO.h"
#include "tehSCREEN.h"
#include "tehBOOP.h"
#include "tehBEEP.h"
#include "tehGUI.h"

/**
 * @brief tehBUS connects all of our interfaces together.
 * 
 * tehBUS was designed as an analogue to what a system bus might look like in a
 *  hypothetical hardware-based CHIP-8 implementation. It is instanced by 
 *  tehCPUS, and it provides methods to read, write, and transfer data between 
 *  our peripherals.
 * 
 * The class accepts pointers to the current back-end implementations of our 
 *  tehSCREEN, tehBEEP, and tehBOOP classes. These pointers are then passed to
 *  the relevant emulated peripherals, which will operate these classes
 *  independently. It may be helpful to visualize this as a video card connected
 *  to a monitor - We are giving tehVIDEO, our video card, access to tehSCREEN,
 *  which is our monitor.
 * 
 * Whenever the bus is clocked with clock_bus(), routines are called to update
 *  the states of the relevant peripheral devices - This allows us to update the
 *  screen, play audio, and handle user events.
 * 
 * @see tehBUS()
 */
class tehBUS {
private:
    tehSCREEN& screen;
    tehBEEP& speaker;
    tehBOOP& keyboard;
    tehGUI& gui;
    tehRAMS* memory;
    tehVIDEO* framebuffer;
    tehAUDIO* audiobuffer;
    bool speakerState;

    chippy::systype system;

public:
    /**
     * @brief The tehBUS class constructor.
     * 
     * Initializes tehBUS, tehRAMS, tehVIDEO, and tehAUDIO.
     * 
     * @param s A pointer to any class that implements tehSCREEN.
     * @param b A pointer to any class that implements tehBEEP.
     * @param k A pointer to any class that implements tehBOOP.
     * @param g A pointer to any class that implements tehGUI.
     * @param sys The value identifying the current quirks mode.
     */
    tehBUS(
          tehSCREEN& s
        , tehBEEP& b
        , tehBOOP& k
        , tehGUI& g
        , chippy::systype sys = chippy::CHIP8
    );
    
    /**
     * @brief The tehBUS class destructor.
     */
    ~tehBUS();

    /**
     * @brief Clocks the system bus.
     * 
     * All peripheral emulation clocks off of this function. If the polling rate
     *  is too low, then the emulator may become unresponsive.
     */
    void clock_bus();

    /**
     * @brief Polls for an exit signal.
     * 
     * @return If true, we should exit, otherwise continue.
     */
    bool get_exit_state();

    // Memory
    
    /**
     * @brief Returns a value from RAM.
     * 
     * @param The address to check.
     * @return The value in ram.
     */
    unsigned char read_ram(int addr);

    /**
     * @brief Writes a value to RAM.
     * 
     * @param addr The address to write to.
     * @param val The data to write.
     */
    void write_ram(int addr, unsigned char val);

    // Video

    /**
     * @brief Clears the framebuffer.
     */
    void blank_screen();
    
    /**
     * @brief Sets video mode in SUPERCHIP quirks mode.
     * 
     * @param mode If True, enables pixel doubling.
     */
    void set_video_mode(bool mode);

    /**
     * @brief Copies sprite data from RAM into the framebuffer.
     * 
     * @param x The X position to copy the sprite to.
     * @param y The Y position to copy the sprite to.
     * @param addr The memory address to copy the sprite from.
     * @param len The size of the sprite.
     */
    bool copy_sprite(int x, int y, short int addr, int len);
    
    // Input

    /**
     * @brief Returns a value representing key that is currently being pressed.
     * 
     * @returns The value of the currently pressed key.
     */
    unsigned char get_key();

    /**
     * @brief Returns whether a given key is currently being pressed.
     * 
     * @param value The key value to test for.
     * @returns True if the key is pressed, otherwise False.
     */
    bool test_key(unsigned char value);

    // Audio

    /**
     * @brief Tells the speaker to beep.
     */
    void screm();
};

#endif