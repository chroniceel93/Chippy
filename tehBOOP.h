/**
 * @file tehBOOP.h
 * @author William Tradewell
 * @brief A virtual interface for handling user input.
 * @version 0.1
 * @date 2026-03-27
 */

#ifndef TEHBOOP_H_
#define TEHBOOP_H_

/**
 * @brief tehBOOP is a virtual interface for handling user input.
 * 
 * This class is a virtual interface that defines a minimum viable set of
 *  functions needed to pass any incoming signals to the larger program.
 */
class tehBOOP {
public:
    // virtual ~tehBOOP() {}

    /**
     * @brief Calls the implementation's event handling loop.
     */
    virtual void process_events() = 0;

    /**
     * @brief Polls for an exit signal.
     * 
     * @returns bool - Exit if true.
     */
    virtual bool get_exit_state() const = 0;

    /**
     * @brief Checks to see if a given key is pressed.
     * 
     * @returns bool - If true, the key is pressed.
     */
    virtual bool is_key_pressed(unsigned char value) const = 0;

    /**
     * @brief Checks to see what key has been pressed.
     * 
     * TODO: This function makes me think that another refactor might be needed.
     * 
     * @returns unsigned char - The first valid keycode.
     */
    virtual unsigned char get_key_pressed() const = 0;
};

#endif