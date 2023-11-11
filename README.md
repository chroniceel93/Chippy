# Chippy!8

In the fall of 2022, I took a programming challenges course, that had me solve several small, byte sized problems. One of those challenges had me write an interpreter.

I graduated that winter.

Not that long after, I decided to copy over some of the rendering boilerplate from a snake game I'd made some years prior, and work on a slightly more complicated system- Though not by much. Just as something to fill the time. Of course, I didn't get very far, before falling ill.

The medication I'm on makes it difficult, to say the least, to do much of anything- Though I do certainly try. Some days, though, I do start to feeling a bit like my old self, and I have a bit more energy than usual. It always turns out that I'd just forgotten to take my AM pills- But I do try to use those incidents for something positive.

This isn't complete. Not by half. My ultimate goal here was to make the emulator functional enough to play a pong rom.

... Yeah, who could have seen it coming. Now that it plays one pong, I want it to play all of the pongs. So- Now the goal of this project mine is to expand the supported instruction set to include Super-Chip8 instructions, and make this thing Just Work with more ROMs.

In its current form, the interpreter resides entirely in the tehCHIP class. (Do not judge my naming scheme. I can name things whatever I want. Nyeh.) Out of everything in the project, it's the most extensively documented, which is good, because it's by far the most complicated.

Incidentally, during the initial creation of this interpreter, I ended up disassembling in its entirety a PONG rom. I do have, at hand, the original notes for that disassembly, and further general notes concerning the architecture of the overall program. I'll make sure to include photocopies of these at some point.

I've got *lots* of to-do's left on this one. In its current state, there's no sound, and I haven't implemented anything to mitigate flickering. I think I'm going to blend frames together to simulate the CRTs these programs were originally displayed on (Simple as save a copy of texture class-wide, re-build and blend each frame? Alternatively, do it in a shader. CPU/GPU tradeoff.). I also want to go back over the SDL code because I really did just copy it from a five year old project. I know that the render-loop's going to dominate the run-time, here- The interpreter's simple as hell, after all- But I suspect that I'm doing something *badly wrong*, there.

Also, as a proof of concept, and so I can have something to copy off of later- I want to set up a build system that can spit out executables for Windows, Mac OS, and Linux. Statically linked, just for fun, if I can.