# Chippy!8

In the fall of 2022, I took a programming challenges course, that had me solve several small, byte sized problems. One of those challenges had me write an interpreter.

I graduated that winter.

Not that long after, I decided to copy over some of the rendering boilerplate from a snake game I'd made some years prior, and work on a slightly more complicated system- Though not by much. Just as something to fill the time. Of course, I didn't get very far, before falling ill.

The medication I'm on makes it difficult, to say the least, to do much of anything- Though I do certainly try. Some days, though, I do start to feeling a bit like my old self, and I have a bit more energy than usual. It always turns out that I'd just forgotten to take my AM pills- But I do try to use those incidents for something positive.

This isn't complete. Not by half. My ultimate goal here was to make the emulator functional enough to play a pong rom.

.. So naturally, I ended up pushing this project until I had a solid interpreter that accurately handled everything in the original Chip-8 spec, quirks and all.

In its current form, the interpreter resides entirely in the tehCHIP class. (Do not judge my naming scheme. I can name things whatever I want. Nyeh.) Out of everything in the project, it's the most extensively documented, which is good, because it's by far the most complicated.

Incidentally, during the initial creation of this interpreter, I ended up disassembling in its entirety a PONG rom. I do have at hand the original notes for that disassembly, and further general notes concerning the architecture of the overall program. I'll make sure to include photocopies of these at some point.

This project has come a long way, but it is by no means done. There are not any menus- ROM loading must be done from the command line, and the emulator cannot be resized. There is no sound, nor is there any way to toggle on, or off the emulation of different quirks. I have not managed to successfully build this on Mac OS, nor have I even bothered to try on Windows. But, as it stands- At least when compiled on linux- The emulator is functional, accurate, and stable- Which is far, far more than I'd planned on once upon a time.

Also, as a proof of concept, and so I can have something to copy off of later- I want to set up a build system that can spit out executables for Windows, Mac OS, and Linux. Statically linked, just for fun, if I can.