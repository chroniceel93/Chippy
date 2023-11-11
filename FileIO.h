/**
 * @file FileIO.cpp
 * @author William Tradewell (Wktradewell@gmail.com)
 * @brief Basic ultily class to handle file access
 * @version 0.9.5
 * @date 2021-01-30
 */

#ifndef FILEIO_H_
#define FILEIO_H_

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

/**
 * @brief FileIO handles basic file input.
 *  
 * It can open and close a read only filestream and
 * read in any arbitrary line.
 */
class FileIO {
protected:
// I set this as protected so any subclasses can have direct access to the 
// fstream object. This allows for a bit more flexibility for them.
    std::fstream file;

private:
    std::string file_name; // filename
    int line_count; // number of lines in the file
    char return_character; // can autodetect this by checking last char in line
    bool read_mode; // 0 = line mode, 1 = chunk mode
/**
 * @brief Counts the number of lines in the file.
 * 
 * By simply iterating on a getline, we can count the number of lines in a file.
 * \n
 * 
 * It's simple, it's probably a lot slower than it could be, but it works!\n
 */
    void countLines();

public:

    FileIO();
    FileIO(std::string input); // input = filename, assume line mode
    
    ~FileIO();

/**
 * @brief Get the File Size object.
 * 
 * @return int Total number of characters in the file.
 */
    int getFileSize();

/**
 * @brief Checks to see if we are trying to read a line that does not exist .
 * 
 * @param line The line we are checking.
 * @return true - We are trying tor ead a line out of bounds!
 * @return false - The line exists, everything is fine!
 */
    bool isOOB(int line);

/**
 * @brief Reads a line in the file.
 * 
 * Using that handy seekLine function, we put the iostream read pointer at the
 * beginning of the line we want to read, and then use getline to read it.
 * 
 * @param line Line number.
 * @return std::string Content of the selected line.
 */
    std::string readLine(int line);

/**
 * @brief Reads the next line in the stream.
 * 
 * Yes, instead of jumping back to the beginning of the file and seeking for-
 * wards until we hit the line we want, we simply go to the next line! Useful
 * for if we need sequential reads!
 * 
 * @return std::string - Content of the next line of data.
 */
    std::string readNextLine();

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
    void writeLine(std::string string, int line); 

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
    void seekLine(int line);

};

#endif