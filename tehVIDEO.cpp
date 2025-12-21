#include "tehVIDEO.h"


tehVIDEO::tehVIDEO(tehSCREEN& s, chippy::systype sys) {
    this->screen = &s;
    this->system = sys;

    this->init_pixel_array();
    this->pixel_doubling = this->system == chippy::SUPERCHIP10 ? true : false;
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
    // Logic for if clipping is off.
    // if (x > 63) {
    //     x = x % 64;
    // }

    // if (y > 31) {
    //     y = y % 32;
    // }

    int pixel_offset = (y * this->fb_width) + x;

    if ((x < this->fb_width) && (y < this->fb_height)) { // test if clipping is on
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
    return true;
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
