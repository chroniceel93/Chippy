#ifndef CHIPPERFLTK_H_
#define CHIPPERFLTK_H_

#include "tehGUI.h"

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>

class chipperFLTK: public tehGUI{
private:
    Fl_Window *main_window;
    Fl_Box *box;
    Fl_Button *paws;

    bool is_exit;
    bool is_paused;
public:
    chipperFLTK();
    ~chipperFLTK();

    void load_rom(std::string filename);

    static void pause_button_callback(Fl_Widget*, void* v);
    bool get_pause_state();

    chippy::systype get_quirks_mode();
    void set_quirks_mode(chippy::systype quirks_mode);

    int get_interpreter_speed();
    void set_interpreter_speed(int interpreter_speed);

    void process_gui_events();
    bool get_exit_state();
} ;

#endif