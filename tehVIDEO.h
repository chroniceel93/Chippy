

#ifndef TEH_VIDEO_H_
#define TEH_VIDEO_H_

#include <cstdlib>

#include "tehCOMMONZ.h"
#include "tehSCREEN.h"

class tehVIDEO {
private:
    tehSCREEN *screen;

    bool *pixel_array;
    bool pixel_doubling;
    chippy::systype system;
    int fb_width;
    int fb_height;
    int fb_size;

    void init_pixel_array();
    void delete_pixel_array();

    bool draw_single_point(int x, int y);
    bool draw_double_point(int x, int y);
    int clip_wrap(int value, int edge);
public:

    tehVIDEO(tehSCREEN& s, chippy::systype sys = chippy::CHIP8);
    ~tehVIDEO();

    void blank_screen();
    bool draw_point(int x, int y);
    void set_video_mode(bool mode);
    void update_screen();

    bool* get_framebuffer();
    int get_framebuffer_height();
    int get_framebuffer_width();
    bool get_video_mode();
};

#endif