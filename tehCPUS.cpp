#include "tehCPUS.h"

using namespace chippy;

/**
 * @brief Construct a new tehCPUS::tehCPUS object
 * 
 * Initializes registers, and RNG.
 * 
 * @param bus pointer to the Chip8 bus object.
 */
tehCPUS::tehCPUS(tehBUS& bus, systype opMode) {
    this->reset();
    this->bus = &bus;
    this->vblank_quirk_block = false;
    this->target = opMode;
    this->dist.param(
        std::uniform_int_distribution<unsigned char>::param_type(0x0, 0xF));
}

/**
 * @brief Return the Nth byte of arbitrarily large words.
 * 
 * Multiplying an index number by 4 (or shifting the binary representation of
 *   such to the left twice), gets us how many bits  Xinto the byte, starting 
 *   from the least significant digit our desired datum is. With that knowledge,
 *   we can then shift our word to the right by X bits, and then mask out the
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

std::string tehCPUS::get_hex_instruction(unsigned short int inst) {
    std::string instruction = "";
    int temp[4];
    temp[0] = this->bitN(inst, 3);
    temp[1] = this->bitN(inst, 2);
    temp[2] = this->bitN(inst, 1);
    temp[3] = this->bitN(inst, 0);
    for (auto i = 0; i < 4; i++) {
        instruction += (temp[i] > 0x9) ? temp[i] + 0x37 : temp[i] + 0x30;
    }
    return instruction;
}

std::string tehCPUS::build_unknown_instruction_error(unsigned short int inst) {
    std::string result = 
        "No instruction 0x" + get_hex_instruction(inst) + " exists";
    return result;
}

bool tehCPUS::is_HP48_family_quirk(systype t) {
    return (t == chippy::CHIP48) && (t == chippy::SUPERCHIP10);
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
        if (!this->haltPC) {
            this->PC += 2;
        } // else, do not iterate PC
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

void tehCPUS::clock_sound() {
    if (this->STreg > 0) {
        this->bus->screm();
    } // else do_nothing()
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
    this->haltPC = false;
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
 *   these instructions, we will call a function that will decode that 
 *   particular subset of instructions.
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
        this->I_1NNN_JMP(inst);
        break;
    case 0x2: // CALL 0xNNN
        this->I_2NNN_CALL(inst);
        break;
    case 0x3: // SE 
        this->I_3XNN_SKIP_IF_EQUAL(inst);
        break;
    case 0x4: // SNE
        this->I_4XNN_SKIP_IF_NOT_EQUAL(inst);
        break;
    case 0x5: // SRE
        this->I_5XY0_SKIP_IF_X_EQ_Y(inst);
        break;
    case 0x6: // LD
        this->I_6XNN_LOAD_NN_TO_X(inst);
        break;
    case 0x7: // ADD
        this->I_7XNN_ADD_NN_TO_X(inst);
        break;
    case 0x8: // Register Ops -
        // (CPR, OR, AND, XOR, ADDR, SUBR, SHR, SUBN, SHL)
        this->decode_hex_8(inst);
        break;
    case 0x9: // SNER
        this->I_9XY0_SKIP_IF_X_NE_Y(inst);
        break;
    case 0xA: // LDI
        this->I_ANNN_LOAD_IREG(inst);
        break;
    case 0xB: // JPR
        this->I_BNNN_JUMP_TO_OFFSET(inst);
        break;
    case 0xC: // RND
        this->I_CXNN_RANDOM(inst);
        break;
    case 0xD: // DRAW
        this->I_DXYN_DRAW(inst);
        break;
    case 0xE: // Key detection OPs - SKP, SKNP
        this->decode_hex_E(inst);
        break;
    case 0xF: // Extra OPs -
        // (RDDT, LDK, LDDT, LDST, ADDI, LDSV, SAVEB, SAVEN, LOADN)
        this->decode_hex_F(inst);
        break;
    default:
        // throw std::out_of_range(build_unknown_instruction_error(inst).c_str());
        std::cout << build_unknown_instruction_error(inst) << std::endl;
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
        this->I_00E0_CLS();
        break;
    case 0x00EE: // RET
        this->I_00EE_RET();
        break;
    case 0x00FE: // DHI
        this->I_00FE_DISABLE_HIRES();
        break;
    case 0x00FF: // HIR
        this->I_00FF_ENABLE_HIRES();
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
        this->I_8XY0_COPY_X_TO_Y(inst);
        break;
    case 0x1: // OR
        this->I_8XY1_OR_X_WITH_Y(inst);
        break;
    case 0x2: // AND
        this->I_8XY2_AND_X_WITH_Y(inst);
        break;
    case 0x3: // XOR
        this->I_8XY3_XOR_X_WITH_Y(inst);
        break;
    case 0x4: // ADDR
        this->I_8XY4_ADD_X_AND_Y(inst);
        break;
    case 0x5: // SUBR
        this->I_8XY5_SUB_Y_FROM_X(inst);
        break;
    case 0x6: // SHR
        this->I_8XZ6_SHIFT_X_RIGHT(inst);
        break;
    case 0x7: // SUBRN
        this->I_8XY7_SUB_X_FROM_Y(inst);
        break;
    case 0xE: // SHL
        this->I_8XZE_SHIFT_X_LEFT(inst);
        break;
    default:
        // Malformed instruction - Throw error
        // throw std::out_of_range(build_unknown_instruction_error(inst).c_str());
        std::cout << build_unknown_instruction_error(inst) << std::endl;
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
        this->I_EX9E_SKIP_IF_KEY(inst);
        break;
    case 0xA1: // SKNP
        this->I_EXA1_SKIP_IF_NO_KEY(inst);
        break;
    default: // NOOP
        // throw std::out_of_range(build_unknown_instruction_error(inst).c_str());
        std::cout << build_unknown_instruction_error(inst) << std::endl;
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
        this->I_FX07_READ_DISPLAY_TIMER(inst);
        break;
    case 0x0A: // LDK
        this->I_FX0A_READ_KEY(inst);
        break;
    case 0x15: // LDDT
        this->I_FX15_SET_DISPLAY_TIMER(inst);
        break;
    case 0x18: // LDST
        this->I_FX18_SET_SOUND_TIMER(inst);
        break;
    case 0x1E: // ADDI
        this->I_FX1E_ADD_VX_TO_I(inst);
        break;
    case 0x29: // LDSV
        this->I_FX29_LOAD_HEX_SPRITE(inst);
        break;
    case 0x33: // SAVEB
        this->I_FX33_SAVE_BCD_VALUE(inst);
        break;
    case 0x55: // SAVEN
        this->I_FX55_SAVE_REGISTERS(inst);
        break;
    case 0x65: // LOADN
        this->I_FX65_LOAD_REGISTERS(inst);
        break;
    default: // NOOP
        // throw std::out_of_range(build_unknown_instruction_error(inst).c_str());
        std::cout << build_unknown_instruction_error(inst) << std::endl;
        break;
    }
}

/**
 * @brief Clears the display.
 * 
 * CLS - (0x00E0).
 */
void tehCPUS::I_00E0_CLS() {
    this->bus->blank_screen();
    return;
}

/**build
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
void tehCPUS::I_00EE_RET() {
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
 * @brief Disables hi-res drawing mode in SUPERCHIP targets.
 */
void tehCPUS::I_00FE_DISABLE_HIRES() {
    if (this->target == chippy::SUPERCHIP10) {
        this->bus->set_video_mode(true);
        // this->bus->set_resolution(64, 32);
    } // else {do_nothing}
    return;
}

/**
 * @brief Enables hi-res drawing mode in SUPERCHIP targets.
 */
void tehCPUS::I_00FF_ENABLE_HIRES() {
    if (this->target == chippy::SUPERCHIP10) {
        this->bus->set_video_mode(false);
        // this->bus->set_resolution(128, 64);
    } // else {do_nothing}
    return;
}

/**
 * @brief Jump PC to location 0x0NNN.
 * 
 * JMP addr - (0x1NNN).
 * 
 * @param inst 
 */
void tehCPUS::I_1NNN_JMP(unsigned short int inst) {
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
void tehCPUS::I_2NNN_CALL(unsigned short int inst) {
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
    this->I_1NNN_JMP(inst);
    return;
}

/**
 * @brief Skip the next instruction if Vx == 0x00NN.
 * 
 * SKIP_IF_EQUAL Vx, 0xNN - (0x3xNN).
 * 
 * Skip the next instruction if the value held in the register X,
 *   (this->regFile[Vx]), where x is given by the third bit from our instruction
 *   (0x0N00), is equal to the valuen given by the firs two bits of our
 *   instruction (0x00NN).
 * 
 * @param inst 
 */
void tehCPUS::I_3XNN_SKIP_IF_EQUAL(unsigned short int inst) {
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

void tehCPUS::I_4XNN_SKIP_IF_NOT_EQUAL(unsigned short int inst) {
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
void tehCPUS::I_5XY0_SKIP_IF_X_EQ_Y(unsigned short int inst) {
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
void tehCPUS::I_6XNN_LOAD_NN_TO_X(unsigned short int inst) {
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
void tehCPUS::I_7XNN_ADD_NN_TO_X(unsigned short int inst) {
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
void tehCPUS::I_8XY0_COPY_X_TO_Y(unsigned short int inst) {
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
void tehCPUS::I_8XY1_OR_X_WITH_Y(unsigned short int inst) {
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
void tehCPUS::I_8XY2_AND_X_WITH_Y(unsigned short int inst) {
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
void tehCPUS::I_8XY3_XOR_X_WITH_Y(unsigned short int inst) {
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
void tehCPUS::I_8XY4_ADD_X_AND_Y(unsigned short int inst) {
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
void tehCPUS::I_8XY5_SUB_Y_FROM_X(unsigned short int inst) {
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
 * SHR Vx - (0x8xZ6).
 * 
 * Shifts the value in Register X to the right by one bit. The least significant
 *   bit is saved to Register F.
 * 
 * Here, we are treating Z as a 'don't care'.
 * 
 * @param inst 
 */
void tehCPUS::I_8XZ6_SHIFT_X_RIGHT(unsigned short int inst) {
    unsigned char regx = this->bitN(inst, 2);
    unsigned char regy = this->bitN(inst, 1);
    // CHIP-8 Quirk: Y is not copied in later interpreters.
    if (this->target == chippy::CHIP8) {
    this->regFile[regx] = this->regFile[regy];
    } // else do_nothing()
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
void tehCPUS::I_8XY7_SUB_X_FROM_Y(unsigned short int inst) {
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
 * @brief Shift the value in Vx left by one - Store MSB in Vf.
 * 
 * SHL Vx - (0x8xZE).
 * 
 * Shifts the value in Register X to the left by one bit. The most significant
 *   bit is saved to Register F.
 * 
 * Here, we are treating Z as a 'Don't care'.
 * 
 * @param inst 
 */
void tehCPUS::I_8XZE_SHIFT_X_LEFT(unsigned short int inst) {
    unsigned char regx = this->bitN(inst, 2);
    unsigned char regy = this->bitN(inst, 1);
    // CHIP-8 Quirk: Y is not copied in later interpreters.
    if (this->target == chippy::CHIP8) {
    this->regFile[regx] = this->regFile[regy];
    }
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
void tehCPUS::I_9XY0_SKIP_IF_X_NE_Y(unsigned short int inst) {
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
void tehCPUS::I_ANNN_LOAD_IREG(unsigned short int inst) {
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
void tehCPUS::I_BNNN_JUMP_TO_OFFSET(unsigned short int inst) {
    if (this->target == chippy::CHIP8) {
    this->PC = this->regFile[0x0] + this->bitsNNN(inst) - 2;
    } else if (this->is_HP48_family_quirk(this->target)) {
        this->PC = this->regFile[(int) this->bitN(inst, 2)] + this->bitsNN(inst) - 2;
    }
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
void tehCPUS::I_CXNN_RANDOM(unsigned short int inst) {
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
 * SUPERCHIP 1.0
 * DRAW Vx, Vy - (0xDxy0) 
 * 
 * This variant will draw a 16x16 size sprite.
 * 
 * @param inst 
 */
void tehCPUS::I_DXYN_DRAW(unsigned short int inst) {
    int len = bitN(inst, 0);
    int fetX = this->regFile[this->bitN(inst, 2)];
    int fetY = this->regFile[this->bitN(inst, 1)];
    int fetI = this->Ireg;

    bool result = false;

    if (len == 0 && this->target == chippy::SUPERCHIP10) {
        result = this->bus->copy_sprite(fetX, fetY, fetI, 16);
    } else {
        result = this->bus->copy_sprite(fetX, fetY, fetI, len);
    }

    if (result) {
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
void tehCPUS::I_EX9E_SKIP_IF_KEY(unsigned short int inst) {
    if (this->bus->test_key(this->regFile[this->bitN(inst, 2)])) {
        this->PC = this->PC + 2;
    } // else, do_nothing();
    return;
}

/**
 * @brief Skip if Key Not Pressed.
 * 
 * SKNP Vx - (0xExA1)
 * 
 * @param inst 
 */
void tehCPUS::I_EXA1_SKIP_IF_NO_KEY(unsigned short int inst) {
    if (!this->bus->test_key(this->regFile[this->bitN(inst, 2)])) {
        this->PC = this-> PC + 2;
    } // else, do_nothing();
    return;
}

/**
 * @brief Save the Delay Timer's current value into Vx.
 * 
 * RDDT Vx - (0xFx07).
 * 
 * @param inst 
 */
void tehCPUS::I_FX07_READ_DISPLAY_TIMER(unsigned short int inst) {
    this->regFile[this->bitN(inst, 2)] = this->DTreg;
    return;
}

/**
 * @brief Save the current key's scancode into the register Vx.
 * 
 * LDK Vx - (0xFx0A).
 * 
 * On LDK, halt interpreter. Wait for key press. If key-press is detected, beep.
 *   beep, and remain halted until key is released, and then resume. Save the 
 *   scancode into Register Vx.
 * 
 * @param inst 
 */
void tehCPUS::I_FX0A_READ_KEY(unsigned short int inst) {
    unsigned char temp = this->bitN(inst, 2);
    // We're doing this a little bit out of order! This is fine.
    // If we read in the keys and *then* test while looping over this instr-
    //  uction, we would overwrite the recorded key every time. 
    if (this->regFile[temp] >= 0x0 && this->regFile[temp] <= 0xF
     && this->haltPC == true) {
        // If the key is still being held, beep and remain halted
        if (this->bus->test_key(this->regFile[temp])) {
            this->STreg = 4;
        } else if (this->STreg == 0) {
            this->haltPC = false;
        }
    } else {
        this->haltPC = true;
        this->regFile[temp] = this->bus->get_key();
    }
    return;
}

/**
 * @brief Load the value in Vx to the Delay Timer Register.
 * 
 * LDDT Vx - (0xFx15).
 * 
 * @param inst 
 */
void tehCPUS::I_FX15_SET_DISPLAY_TIMER(unsigned short int inst) {
    this->DTreg = this->regFile[this->bitN(inst, 2)];
    return;
}

/**
 * @brief Load the value in Vx to the Sound Timer Register.
 * 
 * LDST Vx - (0XFx18).
 * 
 * @param inst 
 */
void tehCPUS::I_FX18_SET_SOUND_TIMER(unsigned short int inst) {
    this->STreg = this->regFile[this->bitN(inst, 2)];
    return;
}

/**
 * @brief Add the value in Vx to I.
 * 
 * ADDI Vx - (0xFx1E).
 * 
 * @param inst 
 */
void tehCPUS::I_FX1E_ADD_VX_TO_I(unsigned short int inst) {
    this->Ireg = this->Ireg + this->regFile[this->bitN(inst, 2)];
    return;
}

/**
 * @brief Load the memory location for the hex sprite of value in Vx.
 * 
 * LDSV Vx - (0xFx29).
 * 
 * @param inst 
 */
void tehCPUS::I_FX29_LOAD_HEX_SPRITE(unsigned short int inst) {
    this->Ireg = this->regFile[this->bitN(inst, 2)] * 5;
    return;
}

/**
 * @brief Save BCD representation of Vx into memory at I(+0-3).
 * 
 * SAVEB x - (0xFx33).
 * 
 * Convert the value stored in register Vx (this->regFile[x]) into a binary-
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
void tehCPUS::I_FX33_SAVE_BCD_VALUE(unsigned short int inst) {
    int tempX = this->regFile[this->bitN(inst, 2)];
    this->bus->write_ram(this->Ireg, ((tempX / 100) % 10));
    this->bus->write_ram(this->Ireg+1, ((tempX / 10) % 10));
    this->bus->write_ram(this->Ireg+2, ((tempX) % 10));
    return;
}

/**
 * @brief Save registers V0 through Vx to memory starting at I.
 * 
 * SAVEN x - (0xFx55).
 * 
 * Save the values of the CPU's main registers (this->regFile[]) in RAM at the
 *   location stored in the Ireg (this->ireg). We will only save the number of
 *   registers equal to x.
 * 
 * @param inst 
 */
void tehCPUS::I_FX55_SAVE_REGISTERS(unsigned short int inst) {
    int tempX = this->bitN(inst, 2);
    for (unsigned char i = 0; i <= tempX; i++) {
        this->bus->write_ram(this->Ireg, this->regFile[i]);
        this->Ireg++;
    }
    // Bug in CHIP48, SUPERCHIP10, SUPERCHIP 11, ireg off by one
    if (this->is_HP48_family_quirk(this->target)) {
        this->Ireg--;
    }
    return;
}

/**
 * @brief Load registers V0 through Vx from memory starting at I.
 * 
 * LOADN x - (0xFx65).
 * 
 * Load the values stored in RAM at the location stored in the Ireg (this->Ireg)
 *   into the CPU's main registers (this->regFile[]). We will only load the num-
 *   ber of registers equal to x.
 * 
 * @param inst 
 */
void tehCPUS::I_FX65_LOAD_REGISTERS(unsigned short int inst) {
    int tempX = this->bitN(inst, 2);
    for (unsigned char i = 0; i <= tempX; i++) {
        this->regFile[i] = this->bus->read_ram(this->Ireg);
        this->Ireg++;
    }
        // Bug in CHIP48, SUPERCHIP10, SUPERCHIP 11, ireg off by one
    if (this->is_HP48_family_quirk(this->target)) {
        this->Ireg--;
    }
    return;
}