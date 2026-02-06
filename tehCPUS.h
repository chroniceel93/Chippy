/**
 * @file tehCPUS.h
 * @author William Tradewell (Wktradewell@gmail.com)
 * @brief Chip-8 Interpreter
 * @version 0.8
 * @date 2023-11-10
 * @copyright Copyright (c) 2023
 */

#ifndef TEHCPUS_H_
#define TEHCPUS_H_

#include <cstring>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

#include "tehBUS.h"
#include "tehCOMMONZ.h"

namespace chippy {
/**
 * @brief tehCPUS Decodes, and Executes Chip-8 instructions.
 * 
 * This class implements the entirety of the original Chip-8 instruction set, 
 *   including quirks as they appeared in the RCA COSMAC VIP. The registers,
 *   counters, and the stack are implemented in this class. All other peripheral
 *   devices can be accessed through the tehBUS class. All memory access occurs
 *   through this BUS class.
 * 
 * Instructions are fetched, decoded, and executed when clock_sys() is called.
 *   This should be called approximately 500 times a second. The clock_60hz() 
 *   function should likewise be called approximately 60 times a second. The
 *   timers can run for at most 4.25 seconds, so drift is not too much of a 
 *   concern.
 * 
 * The halt() function is unimplemented, though the logic for handling the
 *   sprite drawing quirk provides a proof of concept for the idea. It may be
 *   best to refactor this to better expose that functionality.
 * 
 * I do plan on extending the emulator to optionally emulate different sets of
 *   quirks. Ultimately, though, simply emulating the original system is good 
 *   enough for my needs.
 */
class tehCPUS {
private:
    std::default_random_engine generator;
    std::uniform_int_distribution<unsigned char> dist;

    tehBUS* bus;

    bool vblank_quirk_block;
    systype target;
    // If true, PC stops advancing. Instructions that set this to true should
    //   eventually resuem.
    bool haltPC; 

    // REGISTERS
    unsigned char regFile[16];
    // STACK
    unsigned short int stackFile[16];
    // stack pointer
    unsigned char SPreg; 
    // Program counter - pseudo-register- Not accessible from program space
    unsigned short int PC; 
    unsigned short int Ireg;
    // display timer register
    unsigned char DTreg; 
     // sound timer register - play tone when nonzero. Decreses at 60Hz
    unsigned char STreg;

    template<class T>
    unsigned char bitN(T data, int bit);

    template<class T>
    unsigned short int bitsNNN(T data);

    template<class T>
    unsigned short int bitsNN(T data);

    std::string get_hex_instruction(unsigned short int inst);

    // Generates string containing opcode for an unknown instructin
    std::string build_unknown_instruction_error(unsigned short int inst);

    bool is_HP48_family_quirk(systype t);

    void decode_and_execute(unsigned short int inst);

    // 0x0 Block
    void decode_hex_0(unsigned short int inst);
    void I_00E0_CLS  (); // 0x00E0
    void I_00EE_RET  (); // 0x00EE
    void I_00FE_DISABLE_HIRES  (); // 0x00FE
    void I_00FF_ENABLE_HIRES  (); // 0x00FF
    // 0x1 Block
    void I_1NNN_JMP   (unsigned short int inst);
    // 0x2 Block
    void I_2NNN_CALL (unsigned short int inst);
    // 0x3 Block
    void I_3XNN_SKIP_IF_EQUAL   (unsigned short int inst);
    // 0x4 Block
    void I_4XNN_SKIP_IF_NOT_EQUAL  (unsigned short int inst);
    // 0x5 Block
    void I_5XY0_SKIP_IF_X_EQ_Y  (unsigned short int inst);
    // 0x6 Block
    void I_6XNN_LOAD_NN_TO_X   (unsigned short int inst);
    // 0x7 Block
    void I_7XNN_ADD_NN_TO_X  (unsigned short int inst);
    // 0x8 Block
    void decode_hex_8(unsigned short int inst);
    void I_8XY0_COPY_X_TO_Y  (unsigned short int inst);
    void I_8XY1_OR_X_WITH_Y   (unsigned short int inst);
    void I_8XY2_AND_X_WITH_Y  (unsigned short int inst);
    void I_8XY3_XOR_X_WITH_Y  (unsigned short int inst);
    void I_8XY4_ADD_X_AND_Y (unsigned short int inst);
    void I_8XY5_SUB_Y_FROM_X (unsigned short int inst);
    void I_8XZ6_SHIFT_X_RIGHT  (unsigned short int inst);
    void I_8XY7_SUB_X_FROM_Y(unsigned short int inst);
    void I_8XZE_SHIFT_X_LEFT  (unsigned short int inst);
    // 0x9 Block
    void I_9XY0_SKIP_IF_X_NE_Y (unsigned short int inst);
    // 0xA Block
    void I_ANNN_LOAD_IREG  (unsigned short int inst);
    // 0xB Block
    void I_BNNN_JUMP_TO_OFFSET  (unsigned short int inst);
    // 0xC Block
    void I_CXNN_RANDOM  (unsigned short int inst);
    // 0xD Block
    void I_DXYN_DRAW (unsigned short int inst);
    // 0xE Block
    void decode_hex_E(unsigned short int inst);
    void I_EX9E_SKIP_IF_KEY  (unsigned short int inst);
    void I_EXA1_SKIP_IF_NO_KEY (unsigned short int isnt);
    // 0xF Block
    void decode_hex_F(unsigned short int inst);
    void I_FX07_READ_DISPLAY_TIMER (unsigned short int inst);
    void I_FX0A_READ_KEY  (unsigned short int inst);
    void I_FX15_SET_DISPLAY_TIMER (unsigned short int inst);
    void I_FX18_SET_SOUND_TIMER (unsigned short int inst);
    void I_FX1E_ADD_VX_TO_I (unsigned short int inst);
    void I_FX29_LOAD_HEX_SPRITE (unsigned short int inst);
    void I_FX33_SAVE_BCD_VALUE(unsigned short int inst);
    void I_FX55_SAVE_REGISTERS(unsigned short int inst);
    void I_FX65_LOAD_REGISTERS(unsigned short int inst);

public:
    tehCPUS(tehBUS& bus, systype opMode);

    bool halt();

    void clock_sys();

    void clock_60hz();

    void clock_sound();

    void reset();
};
}

#endif