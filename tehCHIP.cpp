#include "tehCHIP.h"

tehCHIP::tehCHIP() {
    this->disk = NULL;
    this->reset_system();
    return;
}

void tehCHIP::load_program(std::string filename) {
    this->disk = new tehROM(filename);
    std::string output = "";
    // NOTE: 0x000-0x1FF reserved for system.
    int current_address = 0x200; // Start of Chip-8 program memory
    do {
        output = this->disk->read_next_chunk();
        for (int i = 0; i < (int) output.size() ; i++) {
            this->bus.write_ram(current_address++, output[i]);
        }
    } while (!this->disk->get_eof());
    // Destroy tehROM class object.
    delete disk;
}

void tehCHIP::execute()  {
    std::chrono::time_point<std::chrono::steady_clock> start, end;
    start = std::chrono::steady_clock::now();
    end = start;
    // resolution of system clock is different from platform to platform
    // auto delta 
    //    = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::chrono::milliseconds delta;

    // while (!this->processor.halt() & !this->screen.update_state()) {
    while (!this->bus.get_exit_state()) {
        delta 
           = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        // TODO: After implementing toggle for vblank quirk, work out some math
        //   to make clock speed easily tweakable. As it stands, this looks
        //   pretty gnarly.
        if (delta > std::chrono::milliseconds(16)) {
            for (auto i = 0; 
                 i < ((1000000 * (int) delta.count()) / 1000000);
                 i++) 
            {
                // SDL_Log("Clock %d", i);
                this->processor.clock_sys();
            }

            this->bus.clock_bus();
            // this->screen.refresh_screen();
            this->processor.clock_60hz();
            start = std::chrono::steady_clock::now();
        } // else, do_nothing();
        this->processor.clock_sound();
        // Sleeping for 0 milliseconds tells the task scheduler that we're
        //   ceding compute time to other processes. 'I can afford to wait'- But
        //   it allows for processing to resume as soon as the task scheduler 
        //   can find free time for it. It makes this a busy-wait loop, without
        //   hogging resources we don't need.
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
        end = std::chrono::steady_clock::now();
    }
    return;
}

void tehCHIP::reset_system()  {
    this->processor.reset();
    // this->memory.clear_tehRAMS();
    // this->screen.blank_screen();
    return;
}