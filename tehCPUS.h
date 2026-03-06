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

/**
 * @brief Return the Nth byte of arbitrarily large words.
 * 
 * @tparam T Type of this function, as determined by data.
 * @param data The data field to be processed.
 * @param bit The byte to be extracted from the datum.
 * @return unsigned char - The requested byte.
 */
    template<class T>
    unsigned char bitN(T data, int bit);

/**
 * @brief Masks all but the least significant three bytes in a word.
 * 
 * @tparam T Type of this function, as determined by data.
 * @param data The data field to be processed.
 * @return unsigned short int - The requested word.
 */
    template<class T>
    unsigned short int bitsNNN(T data);

/**
 * @brief Masks all but the least significant two bytes in a word.
 * 
 * @tparam T Type of this function, as determined by data.
 * @param data The data field to be processed.
 * @return unsigned short int - The requested word.
 */
    template<class T>
    unsigned short int bitsNN(T data);

/**
 * @brief Returns the hexadecimal value of the current instruction as a string.
 * 
 * @param inst The unknown instruction.
 * @return std::string containing a hex value.
 */
    std::string get_hex_instruction(unsigned short int inst);

/**
 * @brief Generates a string containing the opcode for an unknown instruction.
 * 
 * @param inst The unknown instruction.
 * @return std::string containing an appropriately formatted message.
 */
    std::string build_unknown_instruction_error(unsigned short int inst);

/**
 * @brief Checks to see if a given systype should have HP48 family quirks.
 * 
 * @param t The system type we are checking against.
 * @return bool - true if systype is in the HP48 family.
 */
    bool is_HP48_family_quirk(systype t);

/**
 * @brief Decode, and execute an instruction.
 * 
 * @param inst Instruction to decode and execute.
 */
    void decode_and_execute(unsigned short int inst);

    // 0x0 Block
/**
 * @brief Handles all 0x0XXX instructions.
 * 
 * @param inst Instruction to decode and execute.
 */
    void decode_hex_0(unsigned short int inst);

/**
 * @brief Clears the display.
 */
    void I_00E0_CLS();

/**
 * @brief Return from subroutine.
 */
    void I_00EE_RET(); 

/**
 * @brief Disables hi-res drawing mode in SUPERCHIP targets.
 */
    void I_00FE_DISABLE_HIRES();

/**
 * @brief Enables hi-res drawing mode in SUPERCHIP targets.
 */
    void I_00FF_ENABLE_HIRES(); 
    
    // 0x1 Block

/**
 * @brief Jump PC to location 0x0NNN.
 * 
 * @param inst Instruction to execute.
 */
    void I_1NNN_JMP(unsigned short int inst);
    
    // 0x2 Block

/**
 * @brief Call Subroutine at location 0x0NNN.
 * 
 * @param inst Instruction to execute.
 */
    void I_2NNN_CALL(unsigned short int inst);

    // 0x3 Block

/**
 * @brief Skip the next instruction if Vx == 0x00NN.
 * 
 * @param inst Instruction to execute. 
 */
    void I_3XNN_SKIP_IF_EQUAL(unsigned short int inst);
   
    // 0x4 Block

/**
 * @brief Skip the next instruction if Vx != 0x00NN.
 * 
 * @param inst Instruction to execute. 
 */
    void I_4XNN_SKIP_IF_NOT_EQUAL(unsigned short int inst);

    // 0x5 Block

    /**
 * @brief Skip next instruction if Vx == Vy.
 * 
 * @param inst Instruction to execute. 
 */
    void I_5XY0_SKIP_IF_X_EQ_Y(unsigned short int inst);

    // 0x6 Block
    
/**
 * @brief Load value 0xNN into register x.
 * 
 * @param inst Instruction to execute. 
 */
    void I_6XNN_LOAD_NN_TO_X(unsigned short int inst);
    
    // 0x7 Block

/**
 * @brief Add the value 0xNN to the value held in register x.
 * 
 * @param inst Instruction to execute. 
 */
    void I_7XNN_ADD_NN_TO_X(unsigned short int inst);
    
    // 0x8 Block

/**
 * @brief Handles all 0x8XXX instructions.
 * 
 * @param inst Instruction to decode and execute. 
 */
    void decode_hex_8(unsigned short int inst);

/**
 * @brief Copy the value in Vy to Vx.
 *  
 * @param inst Instruction to execute. 
 */
    void I_8XY0_COPY_X_TO_Y(unsigned short int inst);

/**
 * @brief Perform a bitwise OR operation on Vx with Vy.
 * 
 * @param inst Instruction to execute. 
 */
    void I_8XY1_OR_X_WITH_Y(unsigned short int inst);

/**
 * @brief Perform a bitwise AND operation on Vx with Vy.
 * 
 * @param inst Instruction to execute. 
 */
    void I_8XY2_AND_X_WITH_Y(unsigned short int inst);

/**
 * @brief Perform a bitwise XOR operation on Vx with Vy.
 * 
 * @param inst Instruction to execute. 
 */
    void I_8XY3_XOR_X_WITH_Y(unsigned short int inst);

/**
 * @brief Add the value in Vy to Vx - Store carry bit in Vf.
 * 
 * @param inst Instruction to execute. 
 */
    void I_8XY4_ADD_X_AND_Y(unsigned short int inst);

/**
 * @brief Subtract the value in Vy from Vx - Store borrow bit in Vf.
 * 
 * @param inst Instruction to execute. 
 */
    void I_8XY5_SUB_Y_FROM_X(unsigned short int inst);

/**
 * @brief Shift the value in Vx right by one - Store LSB in Vf.
 * 
 * @param inst Instruction to execute. 
 */
    void I_8XZ6_SHIFT_X_RIGHT(unsigned short int inst);

/**
 * @brief Subtract the value in Vx from Vy - Store the borrow in Vf.
 * 
 * @param inst Instruction to execute. 
 */
    void I_8XY7_SUB_X_FROM_Y(unsigned short int inst);

/**
 * @brief Shift the value in Vx left by one - Store MSB in Vf.
 * 
 * @param inst Instruction to execute. 
 */
    void I_8XZE_SHIFT_X_LEFT(unsigned short int inst);

    // 0x9 Block

/**
 * @brief Skip the next instruction if registers Vx and Vy are not equal.
 * 
 * @param inst Instruction to execute. 
 */
    void I_9XY0_SKIP_IF_X_NE_Y(unsigned short int inst);

    // 0xA Block

/**
 * @brief Load 0xNNN into the Ireg.
 * 
 * @param inst Instruction to execute. 
 */
    void I_ANNN_LOAD_IREG(unsigned short int inst);
    
    // 0xB Block

/**
 * @brief Jump to the memory location in V0 + an offset 0xNNN.
 * 
 * @param inst Instruction to execute. 
 */
    void I_BNNN_JUMP_TO_OFFSET(unsigned short int inst);
    
    // 0xC Block

/**
 * @brief Generate a random number.
 * 
 * @param inst Instruction to execute. 
 */
    void I_CXNN_RANDOM(unsigned short int inst);

    // 0xD Block
/**
 * @brief Draw a sprite of length N at (X,Y).
 * 
 * @param inst Instruction to execute. 
 */
    void I_DXYN_DRAW(unsigned short int inst);

    // 0xE Block

/**
 * @brief Handles all 0xEXXX instructions.
 * 
 * @param inst Instruction to decode and execute.
 */
    void decode_hex_E(unsigned short int inst);

/**
 * @brief Skip if Key is Pressed.
 * 
 * @param inst Instruction to execute. 
 */
    void I_EX9E_SKIP_IF_KEY(unsigned short int inst);

/**
 * @brief Skip if Key Not Pressed.
 * 
 * @param inst Instruction to execute. 
 */
    void I_EXA1_SKIP_IF_NO_KEY(unsigned short int isnt);

    // 0xF Block

/**
 * @brief Handles all 0xFXXX instructions.
 * 
 * @param inst Instruction to decode and execute.
 */
    void decode_hex_F(unsigned short int inst);

/**
 * @brief Save the Delay Timer's current value into Vx.
 * 
 * @param inst Instruction to execute. 
 */
    void I_FX07_READ_DISPLAY_TIMER(unsigned short int inst);

/**
 * @brief Save the current key's scancode into the register Vx.
 * 
 * @param inst Instruction to execute. 
 */
    void I_FX0A_READ_KEY(unsigned short int inst);

/**
 * @brief Load the value in Vx to the Delay Timer Register.
 * 
 * @param inst Instruction to execute. 
 */
    void I_FX15_SET_DISPLAY_TIMER(unsigned short int inst);

/**
 * @brief Load the value in Vx to the Sound Timer Register.
 * 
 * @param inst Instruction to execute. 
 */
    void I_FX18_SET_SOUND_TIMER(unsigned short int inst);

/**
 * @brief Add the value in Vx to I.
 * 
 * @param inst Instruction to execute. 
 */
    void I_FX1E_ADD_VX_TO_I(unsigned short int inst);

/**
 * @brief Load the memory location for the hex sprite of value in Vx.
 * 
 * @param inst Instruction to execute. 
 */
    void I_FX29_LOAD_HEX_SPRITE(unsigned short int inst);

/**
 * @brief Save BCD representation of Vx into memory at I(+0-3).
 * 
 * @param inst Instruction to execute. 
 */
    void I_FX33_SAVE_BCD_VALUE(unsigned short int inst);

/**
 * @brief Save registers V0 through Vx to memory starting at I.
 * 
 * @param inst Instruction to execute. 
 */
    void I_FX55_SAVE_REGISTERS(unsigned short int inst);

/**
 * @brief Load registers V0 through Vx from memory starting at I.
 * 
 * @param inst Instruction to execute. 
 */
    void I_FX65_LOAD_REGISTERS(unsigned short int inst);

public:
/**
 * @brief Construct a new tehCPUS::tehCPUS object
 * 
 * Initializes registers, and RNG.
 * 
 * @param bus pointer to the Chip8 bus object.
 */
    tehCPUS(tehBUS& bus, systype opMode);
    
// NO HALTING MECHANISM PRESENT
    bool halt();

/**
 * @brief Main processor clock.
 */
    void clock_sys();

/**
 * @brief Timer register clocks.
 */
    void clock_60hz();

    void clock_sound();

/**
 * @brief Resets the processor.

 */
    void reset();
};
}

#endif