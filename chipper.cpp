#include "chipper.h"

int selftest() {
    return 0; // STUB
}

int main(int argc, char *argv[]) {
    tehCHIP b;
    std::string rom_path;

    if (argc == 1) {
        std::cout << "Rom file not specified.\r\n";
    } else {
        try {
            b.load_program(argv[1]);
            b.execute();
        } catch (const std::out_of_range &e) {
            std::cout << e.what() << std::endl;
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
        }
    }


    return 0;
}