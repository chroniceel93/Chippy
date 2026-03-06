#include "tehCPUS.h"

using namespace chippy;

tehCPUS::tehCPUS(tehBUS& bus, systype opMode) {
    this->reset();
    this->bus = &bus;
    this->vblank_quirk_block = false;
    this->target = opMode;
    this->dist.param(
        std::uniform_int_distribution<unsigned char>::param_type(0x0, 0xF));
}

/* on bitN():
 * Multiplying an index number by 4 (or shifting the binary representation of
 *   such to the left twice), gets us how many bits  Xinto the byte, starting 
 *   from the least significant digit our desired datum is. With that knowledge,
 *   we can then shift our word to the right by X bits, and then mask out the
 *   remainder by performing a bitwise AND on the resultant word. \n 
 *
 * So- Index = bit * 4 == bit << 2  \n 
 *     Byte = (data >> Index) & 0xF 
 */

template<class T>
unsigned char tehCPUS::bitN(T data, int bit) {
    return (data >> (bit << 2)) & 0xF;
}

template<class T>
unsigned short int tehCPUS::bitsNNN(T data) {
    return data & 0x0FFF;
}

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
    // 0x30 is the offset for ASCII numbers.
    // 0x37 is computed by taking 0x41, the offset for ASCII letters, and sub-
    // tracting by 0xA, to account for the letter values starting at 0x9 before
    // addition. 
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


bool tehCPUS::halt() {
    return false;
}

/**
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
 * Set all registers to zero, and set the Program Counter to 0x200.
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

/*
 * First, we decode the instruction using the bitN() function- Working from the
 *   Most significant bit, to the least. In many cases, simply knowing the first
 *   bit is enough to divine exactly what operation to perform. There are excep-
 *   tions to this, however, where other identifying bits must be used. For
 *   these instructions, we will call a function that will decode that 
 *   particular subset of instructions.
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
 * This bank of instruction's are used to handle various system calls. 
 * 
 * Notably, instructions 0x00E0 and 0x00EE are used to clear the screen, and
 *   return from a subroutine, respectively. In addition, all other instructions
 *   are used to run native machine code. This feature is not implimented, and
 *   as such, it will be ignored.
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
 * This bank of instructions are used to handle various register operations. 
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
 * This bank of instructions are used to check if a particular key is pressed
 *   or not.
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
 * This bank of instructions are used to load/store data from various registers,
 *   and the system memory.
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

void tehCPUS::I_00E0_CLS() {
    this->bus->blank_screen();
    return;
}

/**
 * Sets the program counter (this->PC) to the address stored at the top of the
 *   stack (this->stackFile[]), and decrements the Stack Pointer register
 *   (this->SPreg).
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

void tehCPUS::I_00FE_DISABLE_HIRES() {
    if (this->target == chippy::SUPERCHIP10) {
        this->bus->set_video_mode(true);
        // this->bus->set_resolution(64, 32);
    } // else {do_nothing}
    return;
}

void tehCPUS::I_00FF_ENABLE_HIRES() {
    if (this->target == chippy::SUPERCHIP10) {
        this->bus->set_video_mode(false);
        // this->bus->set_resolution(128, 64);
    } // else {do_nothing}
    return;
}

void tehCPUS::I_1NNN_JMP(unsigned short int inst) {
    // We implement PC *after* running the instruction, so we decrement by two
    // to stop an off-by-one. We could probably return a bool to control whether
    // we should clock or not? Eh, this works.
    this->PC = this->bitsNNN(inst) - 2;
    return;
}

/**
 * Save the program counter (this->PC) to the Stack register (this->stackFile[])
 *   at the index of the Stack Pointer (this->SPreg) (So, 
 *   this->stackFile[this->SPreg]), and then increment the Stack Pointer, before
 *   jumping the program counter to the given location.
 * 
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
 * Skip the next instruction if the value held in the register X,
 *   (this->regFile[Vx]), where x is given by the third bit from our instruction
 *   (0x0N00), is equal to the valuen given by the firs two bits of our
 *   instruction (0x00NN).
 */

void tehCPUS::I_3XNN_SKIP_IF_EQUAL(unsigned short int inst) {
    if (this->regFile[this->bitN(inst, 2)] == this->bitsNN(inst)) {
        this->PC = this->PC + 2;
    } // else do_nothing();
    return;
}

/**
 * This is the inverse of I_3XNN_SKIP_IF_EQUAl.
 */

void tehCPUS::I_4XNN_SKIP_IF_NOT_EQUAL(unsigned short int inst) {
    if (this->regFile[this->bitN(inst, 2)] != this->bitsNN(inst)) {
        this->PC = this->PC + 2;
    }
    return;
}

/**
 * Skip the next instruction if the values held in registers X and Y- 
 *   (this->regFile[x], this->regFile[y]) are equal.
 */

void tehCPUS::I_5XY0_SKIP_IF_X_EQ_Y(unsigned short int inst) {
    if (this->regFile[this->bitN(inst, 2)] 
                            == this->regFile[this->bitN(inst, 1)]) {
        this->PC = this->PC + 2;
    }
    return;
}

/**
 * Loads the value 0xNN into register X (this->regFile[X] = 0xNN)
 */

void tehCPUS::I_6XNN_LOAD_NN_TO_X(unsigned short int inst) {
    this->regFile[this->bitN(inst, 2)] = this->bitsNN(inst);
    return;
}

/**
 * Adds the value 0xNN to the value held in register X, and updates that value.
 *   (this->regFile[X] = this->regFile[X] + 0xNN)
 */

void tehCPUS::I_7XNN_ADD_NN_TO_X(unsigned short int inst) {
    unsigned char regx = this->bitN(inst, 2);
    this->regFile[regx] = this->regFile[regx] + this->bitsNN(inst);
    return;
}


void tehCPUS::I_8XY0_COPY_X_TO_Y(unsigned short int inst) {
    this->regFile[this->bitN(inst, 2)] = this->regFile[this->bitN(inst, 1)];
    return;
}

void tehCPUS::I_8XY1_OR_X_WITH_Y(unsigned short int inst) {
    this->regFile[this->bitN(inst, 2)] |= this->regFile[this->bitN(inst, 1)];
// Chip-8 quirk- OR, AND, XOR instructions reset Vf to 0.
    this->regFile[0xF] = 0;
    return;
}

void tehCPUS::I_8XY2_AND_X_WITH_Y(unsigned short int inst) {
    this->regFile[this->bitN(inst, 2)] &= this->regFile[this->bitN(inst, 1)];
// Chip-8 quirk- OR, AND, XOR instructions reset Vf to 0.    
    this->regFile[0xF] = 0;
    return;
}

void tehCPUS::I_8XY3_XOR_X_WITH_Y(unsigned short int inst) {
    this->regFile[this->bitN(inst, 2)] ^= this->regFile[this->bitN(inst, 1)];
// Chip-8 quirk- OR, AND, XOR instructions reset Vf to 0.
    this->regFile[0xF] = 0;
    return;
}

/**
 * Add the values in Registers X and Y together. If the result is greater than 
 *   0xFF, then set Register F to 1, otherwise set it to 0. In both cases, 
 *   save the result to Vx.
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
 * Subtracts the value in Register Y from the value in Register X, and saves the
 *   result to Register X. The borrow flag is defaults to 1 for no borrow, and
 *   is set to 0 if the value in Register X is greater than the value in
 *   Register Y.
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
 * Shifts the value in Register X to the right by one bit. The least significant
 *   bit is saved to Register F.
 * 
 * Here, we are treating Z as a 'don't care'.
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
 * Subtracts the value in Register X from the value in Register Y, and saves the
 *   result to Register X. The borrow flag is defaults to 1 for no borrow, and
 *   is set to 0 if the value in Register X is greater than the value in
 *   Register Y.
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
 * Shifts the value in Register X to the left by one bit. The most significant
 *   bit is saved to Register F.
 * 
 * Here, we are treating Z as a 'Don't care'.
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
 * Compares the values stored in the main registers (this->regFile) Vx, and Vy.
 *   If they are not equal, then we increment the Program Counter.
 */

void tehCPUS::I_9XY0_SKIP_IF_X_NE_Y(unsigned short int inst) {
    if (this->regFile[this->bitN(inst, 2)] 
                                        != this->regFile[this->bitN(inst, 1)]) {
        this->PC = this->PC + 2;
    }
    return;
}

void tehCPUS::I_ANNN_LOAD_IREG(unsigned short int inst) {
    this->Ireg = this->bitsNNN(inst);
    return;
}

/**
 * Because at the end of the clock we increment PC, to fetch the correct
 *   instruction, we must decrement the PC by two.
 * 
 * (this->PC = this->regFile[0] + 0xNNN - 2)
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
 * Generate a random number by using a random number generator- And then 
 *   performing a bitwise AND on the result to mask the result. It seems the
 *   most obvious use for this feature would be to put an upper limit on the 
 *   range of the random value. It is impossible to fully clamp the lower bound.
 * 
 * (this->regFile[x] = RAND() & 0xNN).
 */

void tehCPUS::I_CXNN_RANDOM(unsigned short int inst) {
    this->regFile[this->bitN(inst, 2)] = this->dist(this->generator) 
                                & this->bitsNN(inst);
    return;
}

/**
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

void tehCPUS::I_EX9E_SKIP_IF_KEY(unsigned short int inst) {
    if (this->bus->test_key(this->regFile[this->bitN(inst, 2)])) {
        this->PC = this->PC + 2;
    } // else, do_nothing();
    return;
}

void tehCPUS::I_EXA1_SKIP_IF_NO_KEY(unsigned short int inst) {
    if (!this->bus->test_key(this->regFile[this->bitN(inst, 2)])) {
        this->PC = this-> PC + 2;
    } // else, do_nothing();
    return;
}

void tehCPUS::I_FX07_READ_DISPLAY_TIMER(unsigned short int inst) {
    this->regFile[this->bitN(inst, 2)] = this->DTreg;
    return;
}

/**
 * Halt interpreter. Wait for key press. If key-press is detected, beep.
 *   Beep, and remain halted until key is released, and then resume. Save the 
 *   scancode into Register Vx.
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

void tehCPUS::I_FX15_SET_DISPLAY_TIMER(unsigned short int inst) {
    this->DTreg = this->regFile[this->bitN(inst, 2)];
    return;
}

void tehCPUS::I_FX18_SET_SOUND_TIMER(unsigned short int inst) {
    this->STreg = this->regFile[this->bitN(inst, 2)];
    return;
}

void tehCPUS::I_FX1E_ADD_VX_TO_I(unsigned short int inst) {
    this->Ireg = this->Ireg + this->regFile[this->bitN(inst, 2)];
    return;
}

void tehCPUS::I_FX29_LOAD_HEX_SPRITE(unsigned short int inst) {
    this->Ireg = this->regFile[this->bitN(inst, 2)] * 5;
    return;
}


/**
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
 */

void tehCPUS::I_FX33_SAVE_BCD_VALUE(unsigned short int inst) {
    int tempX = this->regFile[this->bitN(inst, 2)];
    this->bus->write_ram(this->Ireg, ((tempX / 100) % 10));
    this->bus->write_ram(this->Ireg+1, ((tempX / 10) % 10));
    this->bus->write_ram(this->Ireg+2, ((tempX) % 10));
    return;
}

/**
 * Save the values of the CPU's main registers (this->regFile[]) in RAM at the
 *   location stored in the Ireg (this->ireg). We will only save the number of
 *   registers equal to x.
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
 * Load the values stored in RAM at the location stored in the Ireg (this->Ireg)
 *   into the CPU's main registers (this->regFile[]). We will only load the num-
 *   ber of registers equal to x.
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