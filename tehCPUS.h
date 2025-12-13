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

    void decode_and_execute(unsigned short int inst);

    // 0x0 Block
    void decode_hex_0(unsigned short int inst);
    void CLS  (); // 0x00E0
    void RET  (); // 0x00EE
    void DHI  (); // 0x00FE
    void HIR  (); // 0x00FF
    // 0x1 Block
    void JP   (unsigned short int inst);
    // 0x2 Block
    void CALL (unsigned short int inst);
    // 0x3 Block
    void SE   (unsigned short int inst);
    // 0x4 Block
    void SNE  (unsigned short int inst);
    // 0x5 Block
    void SRE  (unsigned short int inst);
    // 0x6 Block
    void LD   (unsigned short int inst);
    // 0x7 Block
    void ADD  (unsigned short int inst);
    // 0x8 Block
    void decode_hex_8(unsigned short int inst);
    void CPR  (unsigned short int inst);
    void OR   (unsigned short int inst);
    void AND  (unsigned short int inst);
    void XOR  (unsigned short int inst);
    void ADDR (unsigned short int inst);
    void SUBR (unsigned short int inst);
    void SHR  (unsigned short int inst);
    void SUBRN(unsigned short int inst);
    void SHL  (unsigned short int inst);
    // 0x9 Block
    void SNER (unsigned short int inst);
    // 0xA Block
    void LDI  (unsigned short int inst);
    // 0xB Block
    void JPR  (unsigned short int inst);
    // 0xC Block
    void RND  (unsigned short int inst);
    // 0xD Block
    void DRAW (unsigned short int inst);
    // 0xE Block
    void decode_hex_E(unsigned short int inst);
    void SKP  (unsigned short int inst);
    void SKNP (unsigned short int isnt);
    // 0xF Block
    void decode_hex_F(unsigned short int inst);
    void RDDT (unsigned short int inst);
    void LDK  (unsigned short int inst);
    void LDDT (unsigned short int inst);
    void LDST (unsigned short int inst);
    void ADDI (unsigned short int inst);
    void LDSV (unsigned short int inst);
    void SAVEB(unsigned short int inst);
    void SAVEN(unsigned short int inst);
    void LOADN(unsigned short int inst);

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