#ifndef TEHRAMS_H_
#define TEHRAMS_H_

class tehRAMS {
private:
    unsigned char memory[4096];
    bool validate_memory_access(int addr);
public:
    tehRAMS();
    void clear_tehRAMS();
    unsigned char read_ram(int addr);
    bool write_ram(int addr, unsigned char val);
};

#endif