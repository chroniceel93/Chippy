/**
 * @file tehSCREEN.h
 * @author William Tradewell
 * @brief A virtual interface for displaying screen data.
 * @version 0.2
 * @date 2026-03-27
 */

#ifndef TEHSCREEN_H_
#define TEHSCREEN_H_

/**
 * @brief tehSCREEN is a virtual interface for displaying screen data
 * 
 * This class is a virtual interface that defines a minimum viable set of
 *  functions needed to control an external video API, and draw our screen.
 */
class tehSCREEN {
private:
    
public:
    virtual ~tehSCREEN() {}

/**
 * @brief Copies screen data from framebuffer to display surface.
 * 
 * Screen data is stored in a boolean array, with false being black, and true
 *  being white. A given implementation should ensure that the size of the data
 *  matches the product of the horizontal and vertical resolutions.
 * 
 * @param data Pointer to beginning of screen data.
 * @param size Length of the screen data.
 */
    virtual void copy_screen(bool* data, int size) = 0;

/**
 * @brief Updates the renderer, and presents it.
 */
    virtual void refresh_screen() = 0;

/**
 * @brief Sets the resolution of the video buffer.
 * 
 * @param w The video buffer's width.
 * @param h The video buffer's height.
 */
    virtual void set_resolution(int w, int h) = 0;

/**
 * @brief Returns the width of the screen.
 * 
 * @return Width of the screen.
 */
    virtual int get_width() = 0;

/**
 * @brief Returns the height of the screen.
 * 
 * @return Height of the screen.
 */
    virtual int get_height() = 0;
};

#endif