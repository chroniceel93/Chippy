/**
 * @file tehSCREEN.h
 * @author William Tradewell
 * @brief SDL2 Display Class.
 * @version 0.1
 * @date 2023-11-04
 */

#ifndef TEHSCREEN_H_
#define TEHSCREEN_H_

// #include "SDL.h"

/**
 * @brief TehSCREEN instances, and performs all operations on, our renderer.
 * 
 * In summary, this class is currently extremely barebones- Implementing Just
 *   Enough boilerplate to provide a display area, and error checking. 
 *   As such, this class is currently very, very limited. We create a single, 
 *   hardware-accelerated window, with a fixed resolution.
 * 
 * Wooohoby do we need to update this!
 */
class tehSCREEN {
private:
    
public:
    virtual ~tehSCREEN() {}

/**
 * @brief Copies screen data from framebuffer to display surface
 */
    virtual void copy_screen(bool* data, int size) = 0;

/**
 * @brief Updates the renderer, and presents it.
 * 
 */
    virtual void refresh_screen() = 0;

/**
 * @brief Sets the resolution of the video buffer.
 */
    virtual void set_resolution(int w, int h) = 0;

    virtual int get_width() = 0;

    virtual int get_height() = 0;
};

#endif