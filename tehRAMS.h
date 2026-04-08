/**
 * @file tehRAMS.h
 * @author William Tradewell
 * @brief This class emulates Chip-8 System Memory.
 * @version 1.0
 * @date 2026-04-208
 */
#ifndef TEHRAMS_H_
#define TEHRAMS_H_

#include <cstddef> // for size_t

/**
 * @class tehRAMS
 * @brief A class for handling emulation of a Chip-8's System Memory.
 */
class tehRAMS {
private:
    /** Pointer to our memory array.*/
    unsigned char *memory;
    /** Size of our memory array. */
    size_t size;

    /**
     * @brief Verifies requested memory address is reachable
     * 
     * This function verifies whether a given memory address exists in our
     *  memory file. If this function returns false, DO NOT ACCESS memory at
     *  that address.
     * 
     * @param addr The address to check.
     * @return True if valid, Flase if invalid.
     */
    bool validate_memory_access(unsigned int addr);

    /**
     * @brief Resets system memory to a clean state.
     * 
     * This routine first writes out the character memory in the first 80 bytes 
     *  before zeroing out the rest of the machine's addressable memory space.
     */
    void clear_tehRAMS();

public:

    /**
     * @brief Initializes the RAM file.
     * 
     * @param size How many bytes to allocate for our system RAM.
     */
    tehRAMS(size_t size = 4096);

    /**
     * @brief Safely destructs our ramfile.
     */
    ~tehRAMS();

    /** 
     * @brief Reads a byte from the RAM file.
     * 
     * In the case that the given address is invalid, we default to simply
     *  returning all 1's (255, 0xFF).
     * 
     * @param addr The address we will read from.
     * @return A single byte of data represented as an unsigned char (8-bits.)
     */
    unsigned char read_ram(unsigned int addr);

    /**
     * @brief Writes a byte to the RAM file.
     * 
     * @param addr The address we will write to.
     * @param val The data we will write.
     * @return False (0) if the write succeeds, otherwise True (1).
     */
    bool write_ram(unsigned int addr, unsigned char val);
};

#endif