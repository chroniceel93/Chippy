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
    // chipperSDL3* sdl;
    // chipperFLTK* fltk;
    tehGUI *gui;
    tehBEEP *bep;
    tehBOOP *bop;
    tehSCREEN *scr;
    chippy::tehCHIP* b;

    bool enableSDL = true;
    bool enableFLTK = true;
    std::string romFileName = "";
    chippy::systype compat = chippy::CHIP8; // we default to Chip-8 compat.
 
    int choice = 0;
    // This loop iterates over every valid argument
    while (true) {
        // int thisOptionOptionIndex = optind ? optind : 1;
        int optionIndex = 0;
        static struct option long_options[] = {
            {"mute",        no_argument,        0,  'm'},
            {"fullscreen",  no_argument,        0,  'f'},
            {"chip48",      no_argument,        0,  'p'},
            {"superchip",   no_argument,        0,  's'},
            {"rom",         required_argument,  0,  'r'},
            {"help",        no_argument,        0,  'h'},
            {"null",        no_argument,        0,  'N'},
            {"sdl_only",    no_argument,        0,  'S'},
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
                compat = chippy::SUPERCHIP10;
                break;
            case 'p':
                compat = chippy::CHIP48;
                break;
            case 'N':
                enableSDL = false;
                enableFLTK = false;
                break;
            case 'S':
                enableSDL = true;
                enableFLTK = false;
                break;
            default:
                // do_nothing();
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

    // New logic, if no file name provided, start with blank interpreter.

    if (romFileName == "") {
        std::cout << "Rom file not specified!\n";
    } else {
        try {
// TODO: Possibly use chipperNULL as a fallback in case SDL fails?
            if (!enableFLTK && !enableSDL) {
/* When we pass the new chipperNULL objec to gui, it is implicitly cast to type
   tehGUI, and so to pass the pointer to chipperNULL onwards, we need to undo
   it with a static cast to the chipperNULL pointer type. We do this with 
   chipperSDL3 too, where appropriate.
*/
                gui = new chipperNULL();
                scr = static_cast<chipperNULL*>(gui);
                bep = static_cast<chipperNULL*>(gui);
                bop = static_cast<chipperNULL*>(gui);
            } else if (!enableFLTK) {
                // Window handling is broken for SDL_Only case
                // Exit events no longer work.
                gui = new chipperNULL();
                scr = new chipperSDL3();
                bep = static_cast<chipperSDL3*>(scr);
                bop = static_cast<chipperSDL3*>(scr);
            } else {
                gui = new chipperFLTK();
                scr = new chipperSDL3();
                bep = static_cast<chipperSDL3*>(scr);
                bop = static_cast<chipperSDL3*>(scr);
            }
            b = new chippy::tehCHIP(*scr, *bep, *bop, *gui, compat);
            // FLTK MUST be initialized before SDL.
            b->load_program(romFileName);
            b->execute();
            std::cout << "Exiting program!" << std::endl;
            delete b;
            // It's overkill
            if (gui != NULL) {
                delete gui;
            }
            if (scr != NULL) {
                delete scr;
            }
            if (bep != NULL) {
                delete bep;
            }
            if (bop != NULL) {
                delete bop;
            }
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
