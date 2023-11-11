#include "tehCPUS.h"

/**
 * @brief Construct a new tehCPUS::tehCPUS object
 * 
 * Initializes registers, and RNG.
 * 
 * @param bus pointer to the Chip8 bus object.
 */
tehCPUS::tehCPUS(tehBUS& bus) {
    this->reset();
    this->bus = &bus;
    this->vblank_quirk_block = false;
    this->dist.param(
        std::uniform_int_distribution<unsigned char>::param_type(0x0, 0xF));
}

/**
 * @brief Return the Nth byte of arbitrarily large words.
 * 
 * Multiplying an index number by 4 (or shifting the binary representation of
 *   such to the left twice), gets us how many bits  Xinto the byte, starting 
 *   from the least significant digit our desired datum is. With that knowledge, we
 *   can then shift our word to the right by X bits, and then mask out the
 *   remainder by performing a bitwise AND on the resultant word. \n 
 *
 * So- Index = bit * 4 == bit << 2  \n 
 *     Byte = (data >> Index) & 0xF 
 * 
 * @tparam T Type of this function, as determined by data.
 * @param data The data field to be processed.
 * @param bit The byte to be extracted from the datum.
 * @return unsigned char - The requested byte.
 */
template<class T>
unsigned char tehCPUS::bitN(T data, int bit) {
    return (data >> (bit << 2)) & 0xF;
}

/**
 * @brief Masks all but the least significant three bytes in a word.
 * 
 * @tparam T Type of this function, as determined by data.
 * @param data The data field to be processed.
 * @return unsigned short int - The requested word.
 */
template<class T>
unsigned short int tehCPUS::bitsNNN(T data) {
    return data & 0x0FFF;
}

/**
 * @brief Masks all but the least significant two bytes in a word.
 * 
 * @tparam T Type of this function, as determined by data.
 * @param data The data field to be processed.
 * @return unsigned short int - The requested word.
 */
template<class T>
unsigned short int tehCPUS::bitsNN(T data) {
    return data & 0x00FF;
}

// NO HALTING MECHANISM PRESENT
bool tehCPUS::halt() {
    return false;
}

/**
 * @brief Main processor clock.
 * 
 * For each cycle, we want to first, fetch the instruction from memory at the
 *   location pointed to by the Program Counter (this->PC). This involves two
 *   reads for the upper half of the instruction, and the lower half of the ins-
 *   truction. Then, we call decode_and_execute, the meat of our interpreter.
 *   After executing, we increment the Program Counter- All jump instructions
 *   take into account this increment, and decrement the PC, effectively holding
 *   it in place for one cycle.
 */
void tehCPUS::clock_sys() {
    if (!this->vblank_quirk_block) {
        short int instruction = this->bus->read_ram(this->PC);
        instruction = (instruction << 8) + this->bus->read_ram(this->PC + 1);
        this->decode_and_execute(instruction);
        this->PC += 2;
    } // else, execution is halted until display refresh.
    return;
}

/**
 * @brief Timer register clocks.
 * 
 * These registers reduce, and are clocked, at a rate of 60 Hz. No operation is 
 *   performed if they're currently zero.
 */
void tehCPUS::clock_60hz() {
    if (this->DTreg > 0) {
        this->DTreg--;
    } // else do_nothing();
    if (this->STreg > 0) {
        this->STreg--;
    } // else do_nothing()
    this->vblank_quirk_block = false;
    return;
}

/**
 * @brief Resets the processor.
 * 
 * Set all registers to zero, and set the Program Counter to 0x200.
 * 
 */
void tehCPUS::reset() {
    this->SPreg = 0;
    this->PC = 0x200;
    this->Ireg = 0;
    this->DTreg = 0;
    this->STreg = 0;
    for (int i = 0; i < 16 ; i++) {
        this->regFile[i] = 0;
        this->stackFile[i] = 0;
    }
    return;
}

/**
 * @brief Decode, and execute an instruction.
 * 
 * First, we decode the instruction using the bitN() function- Working from the
 *   Most significant bit, to the least. In many cases, simply knowing the first
 *   bit is enough to divine exactly what operation to perform. There are excep-
 *   tions to this, however, where other identifying bits must be used. For
 *   these instructions, we will call a function that will decode that particular 
 *   subset of instructions.
 * 
 * @param inst Instruction to decode and execute.
 */
void tehCPUS::decode_and_execute(unsigned short int inst) {
    char err[32];
    unsigned char tmp;
    switch (this->bitN(inst, 3)) {
    case 0x0: // Control OPs - CLS, RET
        this->decode_hex_0(inst);
        break;
    case 0x1: // JP to 0xNNN
        this->JP(inst);
        break;
    case 0x2: // CALL 0xNNN
        this->CALL(inst);
        break;
    case 0x3: // SE 
        this->SE(inst);
        break;
    case 0x4: // SNE
        this->SNE(inst);
        break;
    case 0x5: // SRE
        this->SRE(inst);
        break;
    case 0x6: // LD
        this->LD(inst);
        break;
    case 0x7: // ADD
        this->ADD(inst);
        break;
    case 0x8: // Register Ops -
        // (CPR, OR, AND, XOR, ADDR, SUBR, SHR, SUBN, SHL)
        this->decode_hex_8(inst);
        break;
    case 0x9: // SNER
        this->SNER(inst);
        break;
    case 0xA: // LDI
        this->LDI(inst);
        break;
    case 0xB: // JPR
        this->JPR(inst);
        break;
    case 0xC: // RND
        this->RND(inst);
        break;
    case 0xD: // DRAW
        this->DRAW(inst);
        break;
    case 0xE: // Key detection OPs - SKP, SKNP
        this->decode_hex_E(inst);
        break;
    case 0xF: // Extra OPs -
        // (RDDT, LDK, LDDT, LDST, ADDI, LDSV, SAVEB, SAVEN, LOADN)
        this->decode_hex_F(inst);
        break;
    default:
        strcpy(err, "No instruction 0x");
        tmp = this->bitN(inst, 3);
        err[18] = (tmp > 0x9) ? tmp + 0x41 : tmp + 0x30;
        tmp = this->bitN(inst, 2); 
        err[19] = (tmp > 0x9) ? tmp + 0x41 : tmp + 0x30;
        tmp = this->bitN(inst, 1);
        err[20] = (tmp > 0x9) ? tmp + 0x41 : tmp + 0x30;
        tmp = this->bitN(inst, 0);
        err[21] = (tmp > 0x9) ? tmp + 0x41 : tmp + 0x30;
        strcat(err, " exists");
        throw std::out_of_range(err);
        break;
    }
    return;
}

/**
 * @brief Handles all 0x0XXX instructions.
 * 
 * This bank of instruction's are used to handle various system calls. \n
 * 
 * Notably, instructions 0x00E0 and 0x00EE are used to clear the screen, and
 *   return from a subroutine, respectively. In addition, all other instructions
 *   are used to run native machine code. This feature is not implimented, and
 *   as such, it will be ignored.
 * 
 * @param inst 
 */
void tehCPUS::decode_hex_0(unsigned short int inst) {
    switch (inst) {
    case 0x00E0: // CLS
        this->CLS();
        break;
    case 0x00EE: // RET
        this->RET();
        break;
    default:
        // STUB - Calls subroutine in RCA 1802 microprocessor. Not emulated.
        break;
    }
}

/**
 * @brief Handles all 0x8XXX instructions.
 * 
 * This bank of instructions are used to handle various register operations. 
 * @param inst 
 */
void tehCPUS::decode_hex_8(unsigned short int inst) {
    char err[32];
    unsigned char tmp;
    switch (inst & 0xF) {
    case 0x0: // CPR
        this->CPR(inst);
        break;
    case 0x1: // OR
        this->OR(inst);
        break;
    case 0x2: // AND
        this->AND(inst);
        break;
    case 0x3: // XOR
        this->XOR(inst);
        break;
    case 0x4: // ADDR
        this->ADDR(inst);
        break;
    case 0x5: // SUBR
        this->SUBR(inst);
        break;
    case 0x6: // SHR
        this->SHR(inst);
        break;
    case 0x7: // SUBRN
        this->SUBRN(inst);
        break;
    case 0xE: // SHL
        this->SHL(inst);
        break;
    default:
        // Malformed instruction - Throw error
        strcpy(err, "No instruction 0x8XY");
        tmp = this->bitN(inst, 0);
        err[21] = (tmp > 0x9) ? tmp + 0x41 : tmp + 0x30;
        strcat(err, " exists");
        throw std::out_of_range(err);
        break;
    }
    return;
}

/**
 * @brief Handles all 0xEXXX instructions.
 * 
 * This bank of instructions are used to check if a particular key is pressed
 *   or not.
 * 
 * @param inst 
 */
void tehCPUS::decode_hex_E(unsigned short int inst) {
    char err[32];
    unsigned char tmp;
    switch(this->bitsNN(inst)) {
    case 0x9E: // SKP
        this->SKP(inst);
        break;
    case 0xA1: // SKNP
        this->SKNP(inst);
        break;
    default: // NOOP
        strcpy(err, "No instruction 0xEx");
        tmp = this->bitN(inst, 1);
        err[20] = (tmp > 0x9) ? tmp + 0x41 : tmp + 0x30;
        tmp = this->bitN(inst, 0);
        err[21] = (tmp > 0x9) ? tmp + 0x41 : tmp + 0x30;
        strcat(err, " exists");
        throw std::out_of_range(err);
        break;
    }
    return;
}

/**
 * @brief Handles all 0xFXXX instructions.
 * 
 * This bank of instructions are used to load/store data from various registers,
 *   and the system memory.
 * 
 * @param inst 
 */
void tehCPUS::decode_hex_F(unsigned short int inst) {
    switch(this->bitsNN(inst)) {
    case 0x07: // RDDT
        this->RDDT(inst);
        break;
    case 0x0A: // LDK
        this->LDK(inst);
        break;
    case 0x15: // LDDT
        this->LDDT(inst);
        break;
    case 0x18: // LDST
        this->LDST(inst);
        break;
    case 0x1E: // ADDI
        this->ADDI(inst);
        break;
    case 0x29: // LDSV
        this->LDSV(inst);
        break;
    case 0x33: // SAVEB
        this->SAVEB(inst);
        break;
    case 0x55: // SAVEN
        this->SAVEN(inst);
        break;
    case 0x65: // LOADN
        this->LOADN(inst);
        break;
    default: // NOOP
        break;
    }
}

/**
 * @brief Clears the display.
 * 
 * CLS - (0x00E0).
 */
void tehCPUS::CLS() {
    this->bus->blank_screen();
    return;
}


/**
 * @brief Return from subroutine.
 * 
 * RET - (0x00EE).
 * 
 * Sets the program counter (this->PC) to the address stored at the top of the
 *   stack (this->stackFile[]), and decrements the Stack Pointer register
 *   (this->SPreg).
 * 
 * @param inst 
 */
void tehCPUS::RET() {
    // if SPreg == 0, then do nothing.
    if (this->SPreg == 0) {
        // std::__throw_domain_error("SPreg OOB");
        throw std::out_of_range("Attempted to decrement SPreg OOB!");
        this->SPreg = 0xF;
    } else {
        this->SPreg--;
        this->PC = this->stackFile[this->SPreg];
        this->stackFile[this->SPreg] = 0;
    }
    return;
}

/**
 * @brief Jump PC to location 0x0NNN.
 * 
 * JMP addr - (0x1NNN).
 * 
 * @param inst 
 */
void tehCPUS::JP(unsigned short int inst) {
    // We implement PC *after* running the instruction, so we decrement by two
    // to stop an off-by-one. We could probably return a bool to control whether
    // we should clock or not? Eh, this works.
    this->PC = this->bitsNNN(inst) - 2;
    return;
}


/**
 * @brief Call Subroutine at location 0x0NNN.
 * 
 * CALL addr - (0x2NNN).
 * 
 * Save the program counter (this->PC) to the Stack register (this->stackFile[])
 *   at the index of the Stack Pointer (this->SPreg) (So, 
 *   this->stackFile[this->SPreg]), and then increment the Stack Pointer, before
 *   jumping the program counter to the given location.
 * 
 * @param inst 
 */
void tehCPUS::CALL(unsigned short int inst) {
    // Save current PC to stack
    this->stackFile[this->SPreg] = this->PC;
    // If SPreg > 15 send error
    if (this->SPreg == 0XF) {
        throw std::out_of_range("Attempted to increment SPreg OOB!");
        this->SPreg = 0;
    } else {
        this->SPreg++;
    }
    // Jump 
    this->JP(inst);
    return;
}

/**
 * @brief Skip the next instruction if Vx == 0x00NN.
 * 
 * SE VX, 0xNN - (0x3NN).
 * 
 * Skip the next instruction if the value held in the register X,
 *   (this->regFile[Vx]), where x is given by the third bit from our instruction
 *   (0x0N00), is equal to the valuen given by the firs two bits of our
 *   instruction (0x00NN).
 * 
 * @param inst 
 */
void tehCPUS::SE(unsigned short int inst) {
    if (this->regFile[this->bitN(inst, 2)] == this->bitsNN(inst)) {
        this->PC = this->PC + 2;
    } // else do_nothing();
    return;
}

/**
 * @brief Skip the next instruction if Vx != 0x00NN.
 * 
 * SNE Vx, 0xNN - (0x4xNN).
 * 
 * This is the inverse of the SE instruction. See the SE instruction for 
 *   further details.
 * 
 * @param inst 
 */

void tehCPUS::SNE(unsigned short int inst) {
    if (this->regFile[this->bitN(inst, 2)] != this->bitsNN(inst)) {
        this->PC = this->PC + 2;
    }
    return;
}


/**
 * @brief Skip next instruction if Vx == Vy.
 * 
 * SRE Vx, Vy - (0x5xy0).
 * 
 * Skip the next instruction if the values held in registers X and Y- 
 *   (this->regFile[x], this->regFile[y]) are equal.
 * 
 * @param inst 
 */
void tehCPUS::SRE(unsigned short int inst) {
    if (this->regFile[this->bitN(inst, 2)] 
                            == this->regFile[this->bitN(inst, 1)]) {
        this->PC = this->PC + 2;
    }
    return;
}


/**
 * @brief Load value 0xNN into register x.
 * 
 * LD Vx, 0xNN - (0x6xNN).
 * 
 * Loads the value 0xNN into register X (this->regFile[X] = 0xNN)
 * 
 * @param inst 
 */
void tehCPUS::LD(unsigned short int inst) {
    this->regFile[this->bitN(inst, 2)] = this->bitsNN(inst);
    return;
}

/**
 * @brief Add the value 0xNN to the value held in register x.
 * 
 * ADD Vx, 0xNN - (0x7xNN).
 * 
 * Adds the value 0xNN to the value held in register X, and updates that value.
 *   (this->regFile[X] = this->regFile[X] + 0xNN)
 * 
 * @param inst 
 */
void tehCPUS::ADD(unsigned short int inst) {
    unsigned char regx = this->bitN(inst, 2);
    this->regFile[regx] = this->regFile[regx] + this->bitsNN(inst);
    return;
}

// TODO: Could I do something clever to combine these into one function?
// (Without slowing things down, or causing someone to have nightmares?)

/**
 * @brief Copy the value in Vy to Vx.
 * 
 * CPR Vx, Vy - (0x8xy0).
 * 
 * (this->regFile[x] = this->regFile[y])
 * 
 * @param inst 
 */
void tehCPUS::CPR(unsigned short int inst) {
    this->regFile[this->bitN(inst, 2)] = this->regFile[this->bitN(inst, 1)];
    return;
}

/**
 * @brief Perform a bitwise OR operation on Vx with Vy.
 * 
 * OR Vx, Vy - (0x8xy1).
 * 
 * (this->regFile[x] |= this->regFile[y])
 * 
 * @param inst 
 */
void tehCPUS::OR(unsigned short int inst) {
    this->regFile[this->bitN(inst, 2)] |= this->regFile[this->bitN(inst, 1)];
// Chip-8 quirk- OR, AND, XOR instructions reset Vf to 0.
    this->regFile[0xF] = 0;
    return;
}

/**
 * @brief Perform a bitwise AND operation on Vx with Vy.
 * 
 * AND Vx, Vy - (0x8xy2).
 * 
 * (this->regFile[x] &= this->regFile[y])
 * 
 * @param inst 
 */
void tehCPUS::AND(unsigned short int inst) {
    this->regFile[this->bitN(inst, 2)] &= this->regFile[this->bitN(inst, 1)];
// Chip-8 quirk- OR, AND, XOR instructions reset Vf to 0.    
    this->regFile[0xF] = 0;
    return;
}


/**
 * @brief Perform a bitwise XOR operation on Vx with Vy.
 * 
 * XOR Vx, Vy - (08xy3).
 * 
 * (this->regFile[x] ^= this->regFile[y])
 * 
 * @param inst 
 */
void tehCPUS::XOR(unsigned short int inst) {
    this->regFile[this->bitN(inst, 2)] ^= this->regFile[this->bitN(inst, 1)];
// Chip-8 quirk- OR, AND, XOR instructions reset Vf to 0.
    this->regFile[0xF] = 0;
    return;
}

// ADDR/SUBR instructions broken?

/**
 * @brief Add the value in Vy to Vx - Store carry bit in Vf.
 * 
 * ADDR Vx, Vy - (0x8xy4).
 * 
 * Add the values in Registers X and Y together. If the result is greater than 
 *   0xFF, then set Register F to 1, otherwise set it to 0. In both cases, 
 *   save the result to Vx.
 * 
 * @param inst 
 */
void tehCPUS::ADDR(unsigned short int inst) {
    unsigned char regx = this->bitN(inst, 2);
    unsigned short int temp = ((unsigned short int) this->regFile[regx]) 
                   + ((unsigned short int) this->regFile[this->bitN(inst, 1)]);

    // It is possible to use Register 0xF as the destination. The carry flag 
    // supercedes saving the result, so make sure the carry flag is handled 
    // last.
    this->regFile[regx] = (unsigned char) temp;
    if (temp > 0xFF) {
        this->regFile[0xF] = 1;
    } else {
        this->regFile[0xF] = 0;
    }
    return;
}

/**
 * @brief Subtract the value in Vy from Vx - Store borrow bit in Vf.
 * 
 * SUBR Vx, Vy - (0x8xy5).
 * 
 * Subtracts the value in Register Y from the value in Register X, and saves the
 *   result to Register X. The borrow flag is defaults to 1 for no borrow, and
 *   is set to 0 if the value in Register X is greater than the value in
 *   Register Y.
 * 
 * @param inst 
 */
void tehCPUS::SUBR(unsigned short int inst) {
    unsigned char regx = this->bitN(inst, 2);
    unsigned char regy = this->bitN(inst, 1);
// We must calculate the borrow *before* the subtraction, and apply it *after*
    unsigned char borrow = 1;
    if (this->regFile[regx] < this->regFile[regy]) {
        borrow = 0;
    } // else do_nothing()
    this->regFile[regx] = ((unsigned short int) this->regFile[regx])
                        - ((unsigned short int) this->regFile[regy]);
    this->regFile[0xF] = borrow;
    return;
}

/**
 * @brief Shift the value in Vx right by one - Store LSB in Vf.
 * 
 * SHR Vx - (0x8x*6).
 * 
 * Shifts the value in Register X to the right by one bit. The least significant
 *   bit is saved to Register F.
 * 
 * @param inst 
 */
void tehCPUS::SHR(unsigned short int inst) {
    unsigned char regx = this->bitN(inst, 2);
    unsigned char regy = this->bitN(inst, 1);
    // CHIP-8 Quirk: Y is not copied in later interpreters.
    this->regFile[regx] = this->regFile[regy];
    // Save LSB, before shifting.
    unsigned char LSB = this->regFile[regx] & 0x1;
    this->regFile[regx] >>=1;
    this->regFile[0xF] = LSB;
    return;
}

/**
 * @brief Subtract the value in Vx from Vy - Store the borrow in Vf.
 * 
 * SUBRN Vx, Vy - (0x8xy7).
 * 
 * Subtracts the value in Register X from the value in Register Y, and saves the
 *   result to Register X. The borrow flag is defaults to 1 for no borrow, and
 *   is set to 0 if the value in Register X is greater than the value in
 *   Register Y.
 * 
 * @param inst 
 */
void tehCPUS::SUBRN(unsigned short int inst) {
    unsigned char regx = this->bitN(inst, 2);
    unsigned char regy = this->bitN(inst, 1);
    unsigned char borrow = 1;
    if (this->regFile[regy] < this->regFile[regx]) {
        borrow = 0;
    } // else do_nothing()
    this->regFile[regx] = ((unsigned short int) this->regFile[regy])
                        - ((unsigned short int) this->regFile[regx]);
    this->regFile[0xF] = borrow;
    return;
}


/**
 * @brief Shift the value in Vx by one - Store MSB in Vf.
 * 
 * SHL Vx - (0x8x*E).
 * 
 * Shifts the value in Register X to the left by one bit. The most significant
 *   bit is saved to Register F.
 * 
 * @param inst 
 */
void tehCPUS::SHL(unsigned short int inst) {
    unsigned char regx = this->bitN(inst, 2);
    unsigned char regy = this->bitN(inst, 1);
    // CHIP-8 Quirk: Y is not copied in later interpreters.
    this->regFile[regx] = this->regFile[regy];
    // Save MSB, before shifting.
    unsigned char MSB = (this->regFile[regx] & 0x80) >> 7;
    this->regFile[regx] <<= 1;
    // Write MSB to carry register after shifting.
    this->regFile[0xF] = MSB;
    return;
}


/**
 * @brief Skip the next instruction if registers Vx and Vy are not equal.
 * 
 * Compares the values stored in the main registers (this->regFile) Vx, and Vy.
 *   If they are not equal, then we increment the Program Counter.
 * 
 * SNER Vx, Vy - (0x9xy0);
 * 
 * @param inst 
 */
void tehCPUS::SNER(unsigned short int inst) {
    if (this->regFile[this->bitN(inst, 2)] 
                                        != this->regFile[this->bitN(inst, 1)]) {
        this->PC = this->PC + 2;
    }
    return;
}

/**
 * @brief Load 0xNNN into the Ireg.
 * 
 * LDI 0xNNN - (0xANNN).
 * 
 * @param inst 
 */
void tehCPUS::LDI(unsigned short int inst) {
    this->Ireg = this->bitsNNN(inst);
    return;
}

/**
 * @brief Jump to the memory location in V0 + an offset 0xNNN.
 * 
 * JP 0xNNN - (0xBNNN).
 * 
 * Because at the end of the clock we increment PC, to fetch the correct
 *   instruction, we must decrement the PC by two.
 * 
 * (this->PC = this->regFile[0] + 0xNNN - 2)
 * @param inst 
 */
void tehCPUS::JPR(unsigned short int inst) {
    this->PC = this->regFile[0x0] + this->bitsNNN(inst) - 2;
    return;
}

/**
 * @brief Generate a random number.
 * 
 * RND Vx, NN - (0xCxNN).
 * 
 * Generate a random number by using a random number generator- And then 
 *   performing a bitwise AND on the result to mask the result. It seems the
 *   most obvious use for this feature would be to put an upper limit on the 
 *   range of the random value. It is impossible to fully clamp the lower bound.
 * 
 * (this->regFile[x] = RAND() & 0xNN).
 * @param inst 
 */
void tehCPUS::RND(unsigned short int inst) {
    this->regFile[this->bitN(inst, 2)] = this->dist(this->generator) 
                                & this->bitsNN(inst);
    return;
}

/**
 * @brief Draw a sprite of length N at (X,Y).
 * 
 * DRAW Vx, Vy, N - (0xDxyN).
 * 
 * Copy a sprite to the screen. The I Register (this->Ireg) holds the location
 *   of the sprite to be drawn. The last byte of the instruction tells us how 
 *   many bytes long the sprite data is. We pass all of these to the sprite-
 *   copying function in our BUS object, where the real work is done.
 * 
 * Normally, this would be done manually in here, but offloading the task to a
 *   dedicated function in the BUS object makes sense to me since this is an 
 *   operation that reads from one device, and writes to another.
 * 
 * @param inst 
 */
void tehCPUS::DRAW(unsigned short int inst) {
    if (this->bus->copy_sprite(
        this->regFile[this->bitN(inst, 2)],
        this->regFile[this->bitN(inst, 1)],
        this->Ireg,
        this->bitN(inst, 0)
    )) {
        this->regFile[0xF] = 1;
    } else {
        this->regFile[0xF] = 0;
    }
    this->vblank_quirk_block = true;
    return;
}

/**
 * @brief Skip if Key is Pressed.
 * 
 * SKP Vx - (0xEx9E).
 * 
 * @param inst 
 */
void tehCPUS::SKP(unsigned short int inst) {
    if (this->bus->test_key(this->regFile[this->bitN(inst, 2)])) {
        this->PC = this->PC + 2;
    } // else, do_nothing();
    return;
}

/**
 * @brief Skip if Key Not Pressed.
 * 
 * SKNP Vx - (0xExA1).
 * 
 * @param inst 
 */
void tehCPUS::SKNP(unsigned short int inst) {
    if (!this->bus->test_key(this->regFile[this->bitN(inst, 2)])) {
        this->PC = this-> PC + 2;
    } // else, do_nothing();
    return;
}

/**
 * @brief Save the Delay Timer's current value into VN.
 * 
 * RDDT Vx - (0xFx07).
 * 
 * @param inst 
 */
void tehCPUS::RDDT(unsigned short int inst) {
    this->regFile[this->bitN(inst, 2)] = this->DTreg;
    return;
}

/**
 * @brief Save the current key's scancode into the register VN.
 * 
 * LDK Vx - (0xFx0A).
 * 
 * Instruction is repeated until a scancode is registered, and then the scancode
 *   is saved into Register Vx
 * 
 * @param inst 
 */
void tehCPUS::LDK(unsigned short int inst) {
    unsigned char temp = this->bitN(inst, 2);
    this->regFile[temp] = this->bus->get_key();
    if (this->regFile[temp] > 0xF) {
        this->PC = this->PC - 2;
    }
    return;
}

/**
 * @brief Load the value in VN to the Delay Timer Register.
 * 
 * LDDT Vx - (0xFx15).
 * 
 * @param inst 
 */
void tehCPUS::LDDT(unsigned short int inst) {
    this->DTreg = this->regFile[this->bitN(inst, 2)];
    return;
}

/**
 * @brief Load the value in VN to the Sound Timer Register.
 * 
 * LDST VN - (0XFN18).
 * 
 * @param inst 
 */
void tehCPUS::LDST(unsigned short int inst) {
    this->STreg = this->regFile[this->bitN(inst, 2)];
    return;
}

/**
 * @brief Add the value in VN to I.
 * 
 * ADDI VN - (0xFN1E).
 * 
 * @param inst 
 */
void tehCPUS::ADDI(unsigned short int inst) {
    this->Ireg = this->Ireg + this->regFile[this->bitN(inst, 2)];
    return;
}

/**
 * @brief Load the memory location for the hex sprite of value in VN.
 * 
 * LDSV VN - (0xFN29).
 * 
 * @param inst 
 */
void tehCPUS::LDSV(unsigned short int inst) {
    this->Ireg = this->regFile[this->bitN(inst, 2)] * 5;
    return;
}

/**
 * @brief Save BCD representation of VN into memory at I(+0-3).
 * 
 * SAVEB N - (0xFN33).
 * 
 * Convert the value stored in register VN (this->regFile[N]) into a binary-
 *   coded-decimal, and store it in RAM at address I- Where the most significant
 *   digit is written first, and the subsequent digits are written to I+1, and
 *   I + 2.
 * 
 * To get the 100's digit, divide by 100 (so that X00/100 = x), and modulo by
 *   10.
 * 
 * To get the 10's digit, dividy by 100, and modulo by 10- And finally, for the
 *   1's digit, simply modulo once more by ten. These digits taken together are
 *   the decimal encoding of the hex value.
 * 
 * @param inst 
 */
void tehCPUS::SAVEB(unsigned short int inst) {
    int tempX = this->regFile[this->bitN(inst, 2)];
    this->bus->write_ram(this->Ireg, ((tempX / 100) % 10));
    this->bus->write_ram(this->Ireg+1, ((tempX / 10) % 10));
    this->bus->write_ram(this->Ireg+2, ((tempX) % 10));
    return;
}

/**
 * @brief Save registers V0 through VN to memory starting at I.
 * 
 * SAVEN N - (0xFN55).
 * 
 * Save the values of the CPU's main registers (this->regFile[]) in RAM at the
 *   location stored in the Ireg (this->ireg). We will only save the number of
 *   registers equal to N.
 * 
 * @param inst 
 */
void tehCPUS::SAVEN(unsigned short int inst) {
    int tempX = this->bitN(inst, 2);
    for (unsigned char i = 0; i <= tempX; i++) {
        this->bus->write_ram(this->Ireg, this->regFile[i]);
        this->Ireg++;
    }
    return;
}

/**
 * @brief Load registers V0 through Vx from memory starting at I.
 * 
 * LOADN N - (0xFN65).
 * 
 * Load the values stored in RAM at the location stored in the Ireg (this->Ireg)
 *   into the CPU's main registers (this->regFile[]). We will only load the num-
 *   ber of registers equal to N.
 * 
 * @param inst 
 */
void tehCPUS::LOADN(unsigned short int inst) {
    int tempX = this->bitN(inst, 2);
    for (unsigned char i = 0; i <= tempX; i++) {
        this->regFile[i] = this->bus->read_ram(this->Ireg);
        this->Ireg++;
    }
    return;
}