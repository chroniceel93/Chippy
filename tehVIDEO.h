/**
 * @file tehVIDEO.h
 * @author William Tradewell
 * @brief This class provides our framebuffer emulation.
 * @version 0.1
 * @date 2026-03-30
 */

#ifndef TEH_VIDEO_H_
#define TEH_VIDEO_H_

#include <cstdlib>

#include "tehCOMMONZ.h"
#include "tehSCREEN.h"

class tehVIDEO {
private:
    tehSCREEN *screen;

    // This array of pixels acts as our vram.
    bool *pixel_array;
    // Set to true to enable pixel doubling in SUPERCHIP quirks mode.
    bool pixel_doubling;
    // Determines our quirks mode.
    chippy::systype system;
    // Framebuffer width.
    int fb_width;
    // Framebuffer height.
    int fb_height;
    // Framebuffer size - Calculated off of width and height.
    int fb_size;

    /**
     * @brief Internal utility function that allocates our framebuffer.
     */
    void init_pixel_array();

    /**
     * @breif Internal utility functiont hat deallocates our framebuffer.
     */
    void delete_pixel_array();

    /**
     * @brief Internal utility function that draws one stride of a sprite.
     * 
     * This function is called by draw_sprite() to draw a single byte of data.
     * 
     * @param x The dsired X position.
     * @param y The desired Y position.
     * @param byte The data to draw.
     * @return If True, we have collided with sprite data.
     */
    bool draw_byte(int x, int y, unsigned char byte);

    /**
     * @brief Internal utility function that draws a point.
     * 
     * This function is called by draw_byte to draw a single point in the
     *  framebuffer. Here we determine whether pixel doubling is needed or not.
     * 
     * @param x The desired X position.
     * @param y The desired Y position.
     * @return If True, we have collided with sprite data.
     */
    bool draw_point(int x, int y);

    /**
     * @brief Internal utility function that draws a single point.
     * 
     * This function is called by draw_point and draw_double_point. It handles
     *  sprite clipping, and actually draws, and tests the pixel in the frame-
     *  buffer.
     * 
     * @param x The desired X position.
     * @param y The desired Y position.
     * @return If True, we have collided with sprite data.
     */
    bool draw_single_point(int x, int y);

    /**
     * @brief Internal utility function that draws a size doubled point.
     * 
     * This function is called by draw_point in SUPERCHIP quirks mode, when
     *  sprite doubling is enabled. Internally, it calls draw_single_point() 4
     *  times to draw larger pixels.
     * 
     * @param x The desired x position.
     * @param y The desired y position.
     * @return If True, we have collided with srpite data.
     */
    bool draw_double_point(int x, int y);

    /**
     * @brief Internal utility function that handles wrapping logic for sprites.
     */
    int apply_wrapping_logic(int value, int edge);

    /**
     * @brief Internal utility function that handles clipping logic for sprites.
     */
    int apply_clipping_logic(int value, int edge);
public:
    /**
     * @brief Initializes our internal framebuffer, and our video output.
     * @param s A pointer to a tehSCREEN interface.
     * @param sys A variable holding the current quirks mode.
     */
    tehVIDEO(tehSCREEN& s, chippy::systype sys = chippy::CHIP8);

    /**
     * @brief Safely destructs the framebuffer.
     */
    ~tehVIDEO();

    /**
     * @brief Sets every value in the framebuffer to black.
     */
    void blank_screen();

    /**
     * @brief Toggles pixel doubling mode in SUPERCHIP quirks mode.
     * 
     * Pixel doublign does nothing outside of SUPERCHIP quirks mode.
     * 
     * @param mode If True, we double pixels, otherwise we do not.
     */
    void set_video_mode(bool mode);

    /**
     * @brief Returns whether we are in pixel doubling mode.
     * 
     * @return True if pixel doubling is enabled, otherwise False.
     */
    bool get_video_mode();

    /**
     * @brief Copies the framebuffer to our tehSCREEN interface for display.
     */
    void update_screen();

    /**
     * @brief Draws a sprite to the framebuffer.
     * 
     * NOTE: We can probably copy by reference, here.
     * 
     * This function will draw a sprite to the framebuffer, respecting clipping,
     *  and the current quirks mode.
     * 
     * If we are in SUPERCHIP hi-res mode, then a sprite with a length of 16
     *  will draw a 16x16 sprite.
     * 
     * This function will return false if we 'collide' with existing sprite
     *  data - Flipping a pixel from white, to black.
     * 
     * @param x The desired X position of our sprite.
     * @param y The desired Y position of our sprite.
     * @param size The number of lines to draw our sprite across.
     * @param memory An array containing sprite data to copy from.
     * @return If True, we have collided with sprite data.
     */
    bool draw_sprite( int x, int y, int size, unsigned char (&memory)[32]);

    // The framebuffer getters don't seem to be in use, anywhere.

    /**
     * @brief Returns a pointer to the framebuffer array.
     * 
     * 
     * @returns A pointer to a boolean array holding the framebuffer data.
     */
    bool* get_framebuffer();

    /**
     * @brief Returns the framebuffer height.
     * 
     * @returns The height of the framebuffer.
     */
    int get_framebuffer_height();

    /**
     * @brief Returns the framebuffer width.
     * 
     * @returns The width of the framebuffer.
     */
    int get_framebuffer_width();
};

#endif