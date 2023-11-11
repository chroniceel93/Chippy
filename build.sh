#!/bin/bash
# g++ -Wall -pedantic -Wextra -Og -g -ggdb -fsanitize=undefined,address $(pkg-config sdl2 --cflags --libs) *.cpp
g++ -O2 -s $(sdl2-config --cflags --libs) *.cpp
./a.out "3-corax+.ch8"
./a.out "4-flags.ch8"
./a.out "5-quirks.ch8"