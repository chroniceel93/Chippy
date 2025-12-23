#include "tehVIDEO.h"


tehVIDEO::tehVIDEO(tehSCREEN& s, chippy::systype sys) {
    this->screen = &s;
    this->system = sys;

    this->init_pixel_array();
    this->pixel_doubling = (this->system == chippy::SUPERCHIP10) ? true : false;
}

tehVIDEO::~tehVIDEO() {
    this->delete_pixel_array();
}

// Make note this inits pixel array values, do not blank screen twice
void tehVIDEO::init_pixel_array() {
    switch (this->system) {
        case chippy::CHIP8:
            this->fb_height = 32;
            this->fb_width = 64;
            break;
        case chippy::CHIP48:
            this->fb_height = 32;
            this->fb_width = 64;
            break;
        case chippy::SUPERCHIP10:
            this->fb_height = 64;
            this->fb_width = 128;
            break;
    }
    this->screen->set_resolution(this->fb_width, this->fb_height);
    this->fb_size = sizeof(bool) * this->fb_height * this->fb_width;
    this->pixel_array = (bool*) malloc(this->fb_size);
    this->blank_screen(); // IMMEDIATELY init values
    return;
}

void tehVIDEO::delete_pixel_array() {
    free(this->pixel_array);
    this->pixel_array = nullptr;
    return;
}

void tehVIDEO::blank_screen() {
    for (int i = 0; i < this->fb_size; i++) {
        this->pixel_array[i] = false;
    }
    return;
}

// Why x + 6 - shift, when shift gets up to 7?
// x = 0. +6 = 6. 6-0 = 6. 6-1=5. 6-2=4. 6-3=3. 6-4=2. 6-5=1. 6-6=0.
// ... *OH WAIT*. len is variable. Duh.
// I was accidentally hardcoding the correct solution for a single case.
bool tehVIDEO::draw_sprite(int x, int y, int size, unsigned char (&memory)[16]) {
    bool flipped = false;
    
    // If pixel doubling is in effect, set scaling factor.
    int scaling = this->pixel_doubling ? 2 : 1;

    // Apply scaling factor to screen dimensions
    int width = this->fb_width / scaling;
    int height = this->fb_height / scaling;

    int xpos = this->apply_wrapping_logic(x, width);
    int ypos = this->apply_wrapping_logic(y, height);
    
    unsigned char byte = ' ';
    int shift = 0;
    // for each byte
        // read in the byte
        // Then iterate over the line, until there are no more high bits
            // If the highest bit is high, then it is part of the sprite
    for (int i = 0 ; i < size ; i++) {
        shift = 7;
        byte = memory[i];
        while (byte > 0) {
            if (byte & 0x1) { 
                if (this->draw_point(xpos + shift, ypos + i)) {
                    flipped = true;
                }
            }
            // Shift once, and loop.
            shift--;
            byte = byte >> 1;
        }
    }

    return flipped;
}

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

 * @param x - X coordinate
 * @param y  - Y coordinate
 * @return true - White pixel was flipped
 * @return false - White pixel was not flipped.
 */
bool tehVIDEO::draw_point(int x, int y) {
    bool flipped = false;
    if (this->pixel_doubling) {
        flipped = this->draw_double_point(x, y);
    } else {
        flipped = this->draw_single_point(x, y);
    }
    return flipped;
}

bool tehVIDEO::draw_single_point(int x, int y) {
    bool flipped = false;
    int xpos = x;
    int ypos = y;

    // Clip_wrap will set coords to -1 if clipping is on. In such cases, do not
    // draw the pixel.
    if (xpos > this->fb_width - 1) {
        xpos = this->apply_clipping_logic(xpos, this->fb_width);
    }

    if (y > this->fb_height - 1) {
        ypos = this->apply_clipping_logic(ypos, this->fb_height);
    }

    if ((xpos > -1) && (ypos > -1)) { 
        int pixel_offset = (ypos * this->fb_width) + xpos;
        if (this->pixel_array[pixel_offset] == true) {
            this->pixel_array[pixel_offset] = false;
            flipped = true;
        } else {
            this->pixel_array[pixel_offset] = true;
        }
    } // else, do_nothing();
    return flipped;
}


/**
 * @brief pixel doubler.
 */
bool tehVIDEO::draw_double_point(int x, int y) {
    bool flipped = false;
    int bigx = x * 2;
    int bigy = y * 2;
    // I could make this a loop, but why would I?
    bool pixel_one = draw_single_point(bigx, bigy);
    bool pixel_two = draw_single_point(bigx + 1, bigy);
    bool pixel_three = draw_single_point(bigx, bigy + 1);
    bool pixel_four = draw_single_point(bigx + 1, bigy + 1);

    if (pixel_one || pixel_two || pixel_three || pixel_four) {
        flipped = true;
    }
    return flipped;
}

int tehVIDEO::apply_wrapping_logic(int value, int edge) {
    int result = 0;
    if (value > edge) {
        result = value % edge;
    } else {
        result = value;
    }
    return result;
}

int tehVIDEO::apply_clipping_logic(int value, int edge) {
    int result = 0;
    if (false) {
        result = value % edge;
    } else {
        result = -1;
    }
    return result;
}

/**
    @brief controls pixel doubling
 **/
void tehVIDEO::set_video_mode(bool mode) {
    this->pixel_doubling = mode;
    return;
}

void tehVIDEO::update_screen() {
    this->screen->copy_screen(this->pixel_array, this->fb_size);
    this->screen->refresh_screen();
    return;
}

bool* tehVIDEO::get_framebuffer() {
    return this->pixel_array;
}

int tehVIDEO::get_framebuffer_height() {
    return this->fb_height;
}

int tehVIDEO::get_framebuffer_width() {
    return this->fb_width;
}

bool tehVIDEO::get_video_mode() {
    return this->pixel_doubling;
}
