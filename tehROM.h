#ifndef TEHROM_H_
#define TEHROM_H_

#include <sys/stat.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

class tehROM {
private:
    static const short int chunkSize = 256;
    long fileSize; // Size of file, in bytes
    int chunkTotal; // Total number of chunks of size chunkSize
    int currentChunk; // Current Chunk.
    std::fstream file;
    std::string fileName;
public:
    tehROM();
    tehROM(std::string filename);

    ~tehROM();

    void read_file_size(std::string filename);

    bool get_eof();

    std::string read_next_chunk();
};

#endif