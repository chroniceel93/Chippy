#include "chipper.h"

void print_help() {
    std::cout << 
"Chippy-8, a simple Chip-8 Interpreter by: William Tradewell." << std::endl <<
"Program usage: ./chippy <args> [rom file]" << std::endl <<
"This is just a placeholder, I'mma fill this out later." << std::endl;
    return;
}

// We're using stat here to verify the file exists.
bool verify_file(std::string filename) {
    struct stat buffer;
    int status = 0;
    int errorCode = 0;
    status = stat(filename.c_str(), &buffer);
    if (status == -1) {
        errorCode = errno;
        std::cout << "stat error for file: " 
                  << filename 
                  << std::endl;
        std::cout << "errno(" 
                  << errorCode 
                  << "): " 
                  << strerror(errorCode) 
                  << std::endl;
    }
    return (status == 0);
}

#ifdef _WIN32
int WinMain(int argc, char *argv[]) {
#else
int main(int argc, char *argv[]) {
#endif
    chipperSDL* sdl;
    chippy::tehCHIP* b;

    std::string romFileName = "";
    chippy::systype compat = chippy::CHIP8; // we default to Chip-8 compat.
 
    int choice = 0;
    // This loop iterates over every valid argument
    while (true) {
        int thisOptionOptionIndex = optind ? optind : 1;
        int optionIndex = 0;
        static struct option long_options[] = {
            {"mute",        no_argument,        0,  'm'},
            {"fullscreen",  no_argument,        0,  'f'},
            {"superchip",   no_argument,        0,  's'},
            {"rom",         required_argument,  0,  'r'},
            {"help",        no_argument,        0,  'h'},
            {0,             0,                  0,  0}
        };
        choice = getopt_long(argc, argv, "mfsr", long_options, &optionIndex);

        // getopt returns -1  when there are no more options, break the loop.
        // Using less than 0 to cover the insane case of an overflow, because
        // why shouldn't I be extra?
        if (choice < 0) {
            break;
        }

        // else evaluate choice switch statement
        switch (choice) {
            /* Refrence from getopt man
            case 0:
                printf("option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg %s", optarg);
                printf("\n");
                break;
            */
            case 'h':
                print_help();
                break;
            case 'r':
                if (verify_file(optarg)) {
                    romFileName = optarg;
                } // else do_nothing();
                break;
            case 's':
                compat = chippy::SUPERCHIP;
                break;
        }
    }

    if (romFileName == "") {
    // If the rom file name is still empty, we can check the next non-valid arg
    // to see if the user might've tacked it on to the end of the argument array
    // Iterate through everything looking for valid files.
        for (auto i = optind; i < argc ; i++) {
            if (verify_file(argv[i])) {
                romFileName = argv[i];
                i = argc;
            }
        }
    } // else do_nothing(), we have a valid file.

    if (romFileName == "") {
        std::cout << "Rom file not specified!\n";
    } else {
        try {
            sdl = new chipperSDL();
            b = new chippy::tehCHIP(*sdl, *sdl, *sdl, compat);
            b->load_program(romFileName);
            b->execute();
            std::cout << "Exiting program!";
            delete b;
            delete sdl;
        } catch (const std::out_of_range &e) {
            std::cout << "Out of range error: " << e.what() << std::endl;
        } catch (const std::exception &e) {
            std::cout << "Exception: " << e.what() << std::endl;
        } catch (const char* &e) {
            std::cout << "Exception: " << e << std::endl;
        }
    }

    return 0;
}
