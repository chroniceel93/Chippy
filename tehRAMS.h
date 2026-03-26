#ifndef TEHRAMS_H_
#define TEHRAMS_H_

#include <cstddef> // for size_t

class tehRAMS {
private:
    unsigned char *memory;
    size_t size;
    bool validate_memory_access(unsigned int addr);

public:
    tehRAMS(size_t size = 4096);
    ~tehRAMS();
    void clear_tehRAMS();
    unsigned char read_ram(unsigned int addr);
    bool write_ram(unsigned int addr, unsigned char val);
};

#endif