#include "chipper.h"
#include <nfd.h>
#include <iostream>

#ifdef _WIN32
int WinMain(int argc, char *argv[]) {
#else
int main(int argc, char *argv[]) {
#endif
    tehCHIP b;
    nfdchar_t *outPath = NULL;
    nfdresult_t result = NFD_OpenDialog("ch8", NULL, &outPath);

    switch (result) {
        case NFD_OKAY:
            try {
                b.load_program(outPath);
                b.execute();
            } catch (const std::out_of_range &e) {
                std::cout << "Out of range error: " << e.what() << std::endl;
            } catch (const std::exception &e) {
                std::cout << "Exception: " << e.what() << std::endl;
            }
            break;
        case NFD_CANCEL:
            std::cout << "User pressed cancel." << std::endl;
            break;
        case NFD_ERROR:
            std::cout << "Error: " << NFD_GetError() << std::endl;
            break;
    }

    // Original main, pre NFD

    // if (argc == 1) {
    //     std::cout << "Rom file not specified.\r\n";
    // } else {
    //     try {
    //         b.load_program(argv[1]);
    //         b.execute();
    //     } catch (const std::out_of_range &e) {
    //         std::cout << e.what() << std::endl;
    //     } catch (const std::exception &e) {
    //         std::cout << e.what() << std::endl;
    //     }
    // }

    return 0;
}