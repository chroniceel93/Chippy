CHIP-48Permalink

Created by Andreas Gustafsson for the HP48 graphing calculators. Posted on comp.sys.handhelds in September 1990, source code here.

This is mostly a re-implementation of CHIP-8, but contains a crucial difference in the bit shifting instructions’ semantics, as well as a bug in FX55/FX65.
New instructions

    BXNN: Jump to address XNN + the value in VX (instead of address NNN + the value in V0). Possibly a bug, see this analysis.

Altered instructions

    BNNN is replaced by BXNN (see above)
    FX55/FX65 no longer increment I correctly; it is incremented by one less than it should. If X is 0, it is not incremented at all, as noted in the CHIPPER assembler documentation
    8XY6/8XYE shift VX and ignore VY

Compatibility notes

    The stack is limited to 16 entries.
    The memory is limited to 4K, from 000 to FFF.
    The first 17 bytes of the program were visible as ASCII charactersi (actually, a modified Latin-1) in the HP48’s interface, and so often contain a jump (ie. the two bytes 12NN, displayed as two block characters) followed by the title of the game and sometimes the author.

SUPER-CHIP 1.0Permalink

Created by Erik Bryntse in 1991 for the HP 48S and HP 48SX graphing calculators, based on CHIP-48. It was announced on comp.sci.handhelds May 16, 1991 (with an errata). Also known as SCHIP and S-CHIP.

Adds a high-resolution mode of 128 x 64 as well as persistent memory in the HP48’s “RPL” memory.
New instructions

    00FD: Exit interpreter
    00FE: Disable high-resolution mode
    00FF: Enable high-resolution mode
    DXY0: Draw 16 x 16 sprite (only if high-resolution mode is enabled)
    FX75: Store V0..VX in RPL user flags (X <= 7)
    FX85: Read V0..VX from RPL user flags (X <= 7)

Altered instructions

Same as CHIP-48, plus:

    FX29: Point I to 5-byte font sprite as in CHIP-8, but if the high nibble in VX is 1 (ie. for values between 10 and 19 in hex) it will point I to a 10-byte font sprite for the digit in the lower nibble of VX (only digits 0-9)

Compatibility notes

    In low-resolution mode (ie. the original 64 x 32), the screen memory should still be represented as 128 x 64 with each “pixel” being represented by 2 x 2 pixels. This means that switching between modes produces no visual effect, and the display is not cleared. In fact, the low-resolution mode is simply a special mode where DXYN’s coordinates are doubled. This has more significance in SUPER-CHIP 1.1.
    Memory was uninitialized and random at startup.
    The final byte is reserved, probably because of an off-by-one error, and utilizing it will crash the program.

SUPER-CHIP 1.1Permalink

An extension to SUPER-CHIP, which adds scrolling instructions. It was announced by Erik Bryntse on May 24, 1991 on comp.sys.handhelds.
New instructions

    00CN: Scroll display N pixels down; in low resolution mode, N/2 pixels
    00FB: Scroll right by 4 pixels; in low resolution mode, 2 pixels
    00FC: Scroll left by 4 pixels; in low resolution mode, 2 pixels
    FX30: Point I to 10-byte font sprite for digit VX (only digits 0-9)

Altered instructions

Same as SUPER-CHIP 1.0, but:

    FX29 works like the regular CHIP-8 font instruction; the larger 10-byte font sprites have their own, dedicated FX30 instruction now
    FX55/FX65 no longer increment I at all.

Compatibility notes

    As in SUPER-CHIP 1.0, the low-resolution (64x32) mode is simply a special mode where DXYN’s coordinates are doubled, and each “pixel” is represented by 2x2 on-screen pixels. This in fact means that using the instruction 00C1, a game is able to scroll a “half-pixel” in low-resolution mode. Read more about this here.
    In high resolution mode, DXYN/DXY0 sets VF to the number of rows that either collide with another sprite or are clipped by the bottom of the screen. The original CHIP-8 interpreter only set VF to 1 if there was a collision.


1. Graphics Changes - COMPLETE
- High resolution mode (128x64)
- Move screen drawing logic and framebuffer into tehRAMS? Or tehBUS (hellno)
- tehVIDEO that handles multiple video modes/scaling?

2. New instructions
- 00CN Scroll display N pixels down; in low resolution mode, N/2 pixels
- 00FB Scroll right by 4 pixels; in low resolution mode, 2 pixels
- 00FC Scroll left by 4 pixels; in low resolution mode, 2 pixels
- 00FD Exits the interpreter
- 00FE disables high res mode - x
- 00FF enables high res mode - x
- BXNN (Replaces BRNN, JPR): Jump to address XNN + the value in VX.
- DXY0 Draw 16x16 sprites (if hi res mode is enabled)
- FX30 Point I to 10-byte font sprite for one digit VX (only digits 0-9)
- FX75 Store V0...VX in RPL user flags (??)
- FX85 Read V0...VX in RPL user flags

3. Quirks
- FN55/FN65 no longer increments I correctly; it is incremented by one less than it should. If X is 0, it is not incremented at all. - x
- 8XY6/8XYE shift VX and ignore VY
- In high resolution mode, DXYn/DXY0 sets VF to the number of rows that either collide with another sprite, or are clipped by the bottom of the screen. The original CHIP-8 interpreter only set VF to 1 if there was a collison.

4. Misc. Changes
- Stack is limited to 16 entries
- Memory is limited to 4k, from 0x000, to 0xFFF
- Low-Resolution mode should have the same size (interanlly) as low-resolution mode, with pixels being 2x2 blocks. The screen should not be cleared when switching, and high-res graphics should remain. Low-res mode is effectively a special mode where DXYN's coordiantes are doubled. (More @ https://github.com/Chromatophore/HP48-Superchip/blob/master/investigations/quirk_display.md) — Looks like the trickiest bit to implement
- Memory is unitiialized and random at startup.
- The final byte of ram is reserved, and utilizing it should cause a crash. 

5. HP48 RPL memory - Persistent, 16 bytes?
- Save as binary blob next to ROM?
