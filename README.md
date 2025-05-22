# Chippy!8

In the fall of 2022, I took a programming challenges course, that had me solve several small, byte sized problems. One of those challenges had me write an interpreter.

I graduated that winter.

Not that long after, I decided to copy over some of the rendering boilerplate from a snake game I'd made some years prior, and work on a slightly more complicated system- Though not by much. Just as something to fill the time. Of course, I didn't get very far, before falling ill.

The medication I'm on makes it difficult, to say the least, to do much of anything- Though I do certainly try. Some days, though, I do start to feeling a bit like my old self, and I have a bit more energy than usual. It always turns out that I'd just forgotten to take my AM pills- But I do try to use those incidents for something positive.

This isn't complete. Not by half. My ultimate goal here was to make the emulator functional enough to play a pong rom.

.. So naturally, I ended up pushing this project until I had a solid interpreter that accurately handled everything in the original Chip-8 spec, quirks and all.

In its current form, the interpreter resides entirely in the tehCHIP class. (Do not judge my naming scheme. I can name things whatever I want. Nyeh.) Out of everything in the project, it's the most extensively documented, which is good, because it's by far the most complicated.

Incidentally, during the initial creation of this interpreter, I ended up disassembling in its entirety a PONG rom. I do have at hand the original notes for that disassembly, and further general notes concerning the architecture of the overall program. I've included scans of these notes for posterity as they are ultimately the foundation upon which this emulator was built.

This project has come a long way, but it is by no means done. There is no way to toggle on, or off the emulation of different quirks, nor can I resize the screen. But, as it stands, the emulator is functional, accurate, and stable- Which is far, far more than I'd planned on once upon a time.

The project has a very basic cmake configuration, but it's not really working for anything but linux at the moment. Now that the project is in a place I'm happy with, I'm going to extend it to work on every platform I can.

## Copyright and Credit

This program is built upon SDL2, under the Zlib license. Without it, it would not beep, or flash, or do any of the neat things I'd otherwise like it to do.

The file selection code is enabled by a wonderfully convenient library 'Native File Dialogue' provided by Michael Labbe under the Zlib license.

## Building the Project (Linux)

Before we begin, make sure you've libSDL2 and libSDL2-mixer installed.

Clone the project as normal.
> git clone https://github.com/chroniceel93/chippy \
> cd chippy

Since we use libnfd, we need to initialize that dependency. I've included it as a git submodule, so it'll download from the original repository.
> git submodule update --init --recursive

Next, we'll make a build directory, and enter it.
> mkdir build \
> cd build

Finally, we run cmake, and make, as standard.
> cmake .. \
> make

When the application starts, you will be prompted to supply a chip8 rom file. Several such files can be found at the Chip-8 Archive: https://johnearnest.github.io/chip8Archive/

Keep in mind, this emulator is written to the _original_ Chip-8 spec, and some ROM files take advantage of or depend on features and functionality present in later revisions.