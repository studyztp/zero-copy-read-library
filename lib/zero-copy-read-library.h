#ifndef ZERO_COPY_READ_LIBRARY_H
#define ZERO_COPY_READ_LIBRARY_H

/*
    * Zero Copy Read Library
    * This library provides a mechanism to read data from a file without copying it,
    * allowing for efficient data processing.
*/

#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/stat.h>

#define ERROR_CODE 1
#define SUCCESS_CODE 0

class ZeroCopyRead {
private:
    int fp;                        // File descriptor (should be an int, not int*)
    size_t file_size;
    size_t current_position;
    void* base_mmap_ptr;
    char* iter_mmap_ptr;

public:
    // Constructor
    explicit ZeroCopyRead(int fp);
    // Destructor
    ~ZeroCopyRead();

    // Fill the buffer with data from the file from the specified offset
    // to the specified size.
    size_t readData(size_t offset, size_t size, void* buffer);

    char operator*() const {
        return *iter_mmap_ptr;
    }

    int operator*() {
        return *reinterpret_cast<int*>(iter_mmap_ptr);
    }

    size_t operator++() {
        if (current_position + 1 >= file_size) {
            return ERROR_CODE; // End of file reached
        }
        iter_mmap_ptr++;
        current_position++;
        return SUCCESS_CODE; // Successfully moved to the next character
    }

    size_t operator--() {
        if (current_position == 0) {
            return ERROR_CODE; // Cannot move back, already at the start
        }
        iter_mmap_ptr--;
        current_position--;
        return SUCCESS_CODE; // Successfully moved to the previous character
    }

    size_t operator+=(size_t offset) {
        if (current_position + offset >= file_size) {
            return ERROR_CODE; // Out of bounds
        }
        iter_mmap_ptr += offset;
        current_position += offset;
        return SUCCESS_CODE; // Successfully moved forward by offset
    }

    size_t operator-=(size_t offset) {
        if (current_position < static_cast<size_t>(offset)) {
            return ERROR_CODE; // Out of bounds
        }
        iter_mmap_ptr -= offset;
        current_position -= offset;
        return SUCCESS_CODE; // Successfully moved backward by offset
    }

    int operator-(const ZeroCopyRead& other) const {
        return *reinterpret_cast<int*>(iter_mmap_ptr) - *reinterpret_cast<int*>(other.iter_mmap_ptr);
    }

    int operator+(const ZeroCopyRead& other) const {
        return *reinterpret_cast<int*>(iter_mmap_ptr) + *reinterpret_cast<int*>(other.iter_mmap_ptr);
    }

    int operator*(const ZeroCopyRead& other) const {
        return *reinterpret_cast<int*>(iter_mmap_ptr) * *reinterpret_cast<int*>(other.iter_mmap_ptr);
    }

    int operator/(const ZeroCopyRead& other) const {
        int right_value = *reinterpret_cast<int*>(other.iter_mmap_ptr);
        if (right_value == 0) {
            throw std::runtime_error("Division by zero");
        }
        return *reinterpret_cast<int*>(iter_mmap_ptr) / right_value;
    }

    size_t getCurrentPosition() const {
        return current_position;
    }

    size_t getFileSize() const {
        return file_size;
    }

    void resetIterator() {
        iter_mmap_ptr = static_cast<char*>(base_mmap_ptr);
        current_position = 0;
    }

};

#endif // ZERO_COPY_READ_LIBRARY_H
