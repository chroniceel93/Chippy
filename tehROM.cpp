#include "tehROM.h"

tehROM::tehROM() {

}

tehROM::tehROM(std::string filename) {
    read_file_size(filename);
    this->currentChunk = 0;
    if (this->fileSize > 0) {
        this->file.open(filename.c_str(), std::ios::binary | std::ios::in);
    } else {
        throw std::range_error("fileSize < 0 - File not found.");
    }
}

tehROM::~tehROM() {
    this->file.close();
}

void tehROM::read_file_size(std::string filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    this->fileSize = rc == 0 ? stat_buf.st_size : -1;
    this->chunkTotal = this->fileSize / this->chunkSize;
}

bool tehROM::get_eof() {
    return (this->currentChunk > this->chunkTotal);
}

std::string tehROM::read_next_chunk() {
    // We know our current position, and the size of the file. This lets us 
    // determine the size of the final chunk
    short int chunk = this->chunkSize;
    if (this->currentChunk >= this->chunkTotal) {
        chunk = this->fileSize % this->chunkSize;
    } // else do_nothing();

    char *readInBuffer = new char[chunk];

    // pre-allocate memory to avoid re-allocating when copying over c-string
    // This is already pretty brute-force, no need to make it any slower than it
    // already will be.
    std::string readInString = "";
    readInString.reserve(chunk); 
    this->file.read(readInBuffer, chunk);
    for (short int i = 0; i < chunk; i++) {
        readInString += readInBuffer[i];
    }
    this->currentChunk++;
    delete[] readInBuffer;
    return readInString;
}
