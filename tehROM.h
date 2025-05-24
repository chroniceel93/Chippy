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
 */
class tehROM {
private:
    static const short int chunkSize = 256; /**< Size of each chunk to be read from the file. */
    long fileSize; /**< Size of the file, in bytes. */
    int chunkTotal; /**< Total number of chunks of size chunkSize. */
    int currentChunk; /**< Index of the current chunk being read. */
    std::fstream file; /**< File stream object used to open and read the file. */
    std::string fileName; /**< Name of the file to be read. */
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
     * @brief Destructor.
     */
    ~tehROM();

    /**
     * @brief Determines the size of the file and calculates the total number of chunks.
     * @param filename The name of the file.
     */
    void read_file_size(std::string filename);

    /**
     * @brief Checks if the end of the file has been reached.
     * @return true if the end of the file has been reached, false otherwise.
     */
    bool get_eof();

    /**
     * @brief Reads the next chunk of data from the file.
     * @return The next chunk as a string.
     */
    std::string read_next_chunk();
};

#endif