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
 * @brief XOR one pixel, and return whether any white pixels were flipped.
 * 
 * The coordinate plane starts at the top left corner. X increases to the right
 *   and Y increases downwards. To evaluate a given pixel, the coordinates must
 *   be converted into an array index. This pixel offset can be found by mult-
 *   iplying the Y-coordinate by the number of pixels per row- And then adding
 *   to that the value of the X-coordinate. More specifically, for the classic
 *   display mode, this works out to (y * 64) + x.
 * 
 * Then, we test the value of the pixel in that address. If it is all hi- Then 
 *   we need to set our return variable to true, and flip it to black. Otherwise,
 *   we set the pixel to white. We do not have, or need, logic to explicitly set
 *   a pixel to black.
 * 
 * NOTICE:
 * 
 * This effect seems to misbehave on MacOS. Looking into it further. It looks 
 *   like textures are being blitted to the render texture out of order. This 
 *   leads to a rather jarring jittering effect, displaying screen data that can 
 *   be several seconds old.
 * 
 * @param x - X coordinate
 * @param y  - Y coordinate
 * @return true - White pixel was flipped
 * @return false - White pixel was not flipped.
 */
    virtual bool draw_point(int x, int y) = 0;

/**
 * @brief Updates the renderer, and presents it.
 * 
 */
    virtual void refresh_screen() = 0;

/**
 * @brief Sets every pixel to opaque black. This will clear the screen.
 * 
 */
    virtual void blank_screen() = 0;

/**
 * @brief Sets the resolution of the video buffer.
 */
    virtual void set_resolution(int w, int h) = 0;

    virtual int get_width() = 0;

    virtual int get_height() = 0;
};

#endif