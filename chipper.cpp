#include "chipper.h"

#include <nfd.h>

#ifdef _WIN32
int WinMain(int argc, char *argv[]) {
#else
int main(int argc, char *argv[]) {
#endif
    tehCHIP b;
    nfdchar_t *outPath = NULL;
    nfdresult_t result = NFD_OpenDialog( "ch8", NULL, &outPath);

    if (result == NFD_OKAY) {
        try {
            b.load_program(outPath);
            b.execute();
        } catch (const std::out_of_range &e) {
            std::cout << e.what() << std::endl;
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
        }
    } // All other cases should exit. We either have a ROM, or we do not.

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
