/**
 * @file FileIO.cpp
 * @author William Tradewell (Wktradewell@gmail.com)
 * @brief 
 * @version 0.9.5
 * @date 2021-01-30
 */


#include "FileIO.h"

// CONSTRUCTORS

FileIO::FileIO () {
    // fully aware this is implied w/o this-> syntax, but I like to specify
    this->line_count = 0;
    this->return_character = '\n';
}

// 2023-10-06 - Notes on fstream failure states
// the goodbit could be set low under many conditions- Here it looks as if
// I assumed that it would simply be because the file does not exist. In ac-
// tuality, the file may be in use by something else. 
// It would've been a bit better to check if the file existed-
// #include <sys/stat.h>
// inline bool exists (const)

FileIO::FileIO(std::string input) {
    this->line_count = 0;
    this->return_character = '\n';
    this->file.open(input, std::ios::in | std::ios::out);
    // useful for checking to see if I've broken output somehow
    // file.open(input, std::ios::in | std::iso::out | std::ios:app);
    if (!file.good()) {
        this->file.close();
        std::ofstream file {input};
        this->file.open(input, std::ios::in | std::ios::out);
    }
    this->file_name = input;
    this->countLines();
    // can't figure out how to make this create a new file
}

// DESTRUCTOR

FileIO::~FileIO () {
    this->file.close();
}

/**
 * @brief Get the File Size object.
 * 
 * @return int Total number of characters in the file.
 */
int FileIO::getFileSize() {
    int result = 0;
    // set pointer to end
    this->file.seekg(0);
    this->file.clear();
    this->file.seekg(0, std::ios::end);
    result = static_cast<int>(this->file.tellg());
    this->file.seekg(this->file.tellp());
    this->file.clear();
    return result;
}

/**
 * @brief Checks to see if we are trying to read a line that does not exist 
 * 
 * @param line The line we are checking.
 * @return true - We are trying tor ead a line out of bounds!
 * @return false - The line exists, everything is fine!
 */
bool FileIO::isOOB(int line) {
    bool result = false;
    if (line > this->line_count) 
        result = true;
    return result;
}

/**
 * @brief Reads a line in the file.
 * 
 * Using that handy seekLine function, we put the iostream read pointer at the
 * beginning of the line we want to read, and then use getline to read it.
 * 
 * @param line Line number.
 * @return std::string Content of the selected line.
 */
std::string FileIO::readLine(int line) {
    std::string readin = "";
    if (this->line_count < line) {
        // impossible line count, something's gone wrong
        // throw std::out_of_range;
        // ehh, this is informative enough
        readin = "Line out of range!\n";
    } else {
        seekLine(line);
        // this does not work in c++ 14? 
        std::getline(file, readin, this->return_character);
    }
    return readin;
}

/**
 * @brief Reads the next line in the stream.
 * 
 * Yes, instead of jumping back to the beginning of the file and seeking for-
 * wards until we hit the line we want, we simply go to the next line! Useful
 * for if we need sequential reads!
 * 
 * @return std::string - Content of the next line of data.
 */
std::string FileIO::readNextLine() {
    std::string readin = "";
    std::getline(file, readin, this->return_character);
    return readin;
}


/**
 * @brief Counts the number of lines in the file.
 * 
 * By simply iterating on a getline, we can count the number of lines in a file.
 * \n
 * 
 * It's simple, it's probably a lot slower than it could be, but it works!\n
 */
void FileIO::countLines() {
    std::string dump = ""; // this one exists to do work and then die :3
    seekLine(0);
    while (std::getline(file, dump, this->return_character)) {
        this->line_count++;
    }
    seekLine(0);
}

/**
 * @brief Writes a line to the file at on a specified line.
 * 
 * This function uses seekLine to find the correct location, then overwrites
 * the data on that line. If it's at the end of the file, then it'll simply
 * append.
 * 
 * @param string The data to be written.
 * @param line The line the data should be written to.
 */
void FileIO::writeLine(std::string string, int line) {
    seekLine(line);
    this->file << string << this->return_character << std::flush;
}

/**
 * @brief Sets iostream pointers to a specified line.
 *  
 * This function iterates through a file by looping a token getline function
 * until a counter reaches the wanted line number. \n
 * 
 * We always seek from the beginning, and this function will always work
 * correctly as long as the class is configured to use the newline character
 * present in the file. \n
 * 
 * This will fail spectacularly if we are looking for the wrong newline char. \n
 * 
 * @param line The line number this should seek to.
 */
void FileIO::seekLine(int line) {
    std::string dump = ""; // temporary variable used for getline to work
// resets iostream status
    this->file.clear();
// moves read pointer to start of file
    this->file.seekg(0, std::ios::beg);
//  loops through file until we hit the correct line number
// there is a reason I'm not using this for the database.
    for (auto i = 0; i < line ; i++) {
        std::getline(file, dump, this->return_character);
    }
    // sets write pointer = read pointer
    this->file.seekp(this->file.tellg());
}