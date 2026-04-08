/**
 * @file tehROM.h
 * @author William Tradewell
 * @brief Here we handle loading the ROM into memory.
 * @version 1.1
 * @date 2026-04-06
 */

#ifndef TEHROM_H_
#define TEHROM_H_

#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

/**
 * @class tehROM
 * @brief A class for reading a file into memory in chunks.
 * 
 * This class allows us to access, and read into memory a ROM file in chunks of
 *  256 bytes. In its current state, it is very simple, only needing to read in
 *  one file, once - But as designed, it should be fairly easy to extend this to
 *  sek back and forth getting data from larger files.
 * 
 * I might be able to repurpose this later into saving, and loading a system 
 *  state.
 */
class tehROM {
private:
    /**< Size of each chunk to be read from the file. */
    static const short int chunkSize = 256;
    /**< Size of the file, in bytes. */
    long fileSize;
    /**< Total number of chunks of size chunkSize. */
    int chunkTotal;
    /**< Index of the current chunk being read. */
    int currentChunk;
    /**< File stream object used to open and read the file. */
    std::fstream file; 
    /**< Name of the file to be read. */
    std::string fileName; 

    /**
     * @brief Determines the size of a file.
     * @param filename The name of the file.
     */
    void read_file_size(std::string filename);
public:
    /**
     * @brief Default constructor.
     */
    tehROM();

    /**
     * @brief Constructor that initializes the object with a file.
     * @param filename The name of the file to be read.
     */
    tehROM(std::string filename);

    /**
     * @brief Destructor that closes the open file.
     */
    ~tehROM();

    /**
     * @brief Checks if the end of the file has been reached.
     * @return True if the end of the file has been reached, otherwise False.
     */
    bool get_eof();

    /**
     * @brief Reads the next chunk of data from the file.
     * @return The next chunk as a string.
     */
    std::string read_next_chunk();
};

#endif