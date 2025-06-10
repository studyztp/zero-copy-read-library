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
#include <atomic>
#include <cstring>
#include <thread>
#include <chrono>

#define ERROR_CODE 1
#define SUCCESS_CODE 0
#define MAX_BUFFER_SIZE 1024

class ZeroCopyRead {
private:
    int fd;                        // File descriptor (should be an int, not int*)
    int lock_fd;                // File descriptor for the lock file
    size_t file_size;
    size_t current_position;
    void* base_mmap_ptr;
    char* iter_mmap_ptr;
    void* lock_mmap_ptr;
    std::string file_path_;
    std::string lock_file_path_;
    
    alignas(64) char shared_buffer[MAX_BUFFER_SIZE];
    std::atomic<bool> ready;

public:
    // Constructor
    explicit ZeroCopyRead(const char* file_path, const char* lock_file_path);
    // Destructor
    ~ZeroCopyRead();

    size_t atomicReadLine(char* buffer);

    void readLockfile();

    size_t checkFileValidity(int fd) const {
        struct stat file_stat;
        if (fstat(fd, &file_stat) == -1) {
            return ERROR_CODE; // Failed to get file status
        }
        return SUCCESS_CODE; // File is valid
    }

    size_t openFile(int* fd, const char* file_path) {
        *fd = open(file_path, O_RDONLY);
        if (*fd == -1) {
            perror("Failed to open file");
            return ERROR_CODE; // Failed to open file
        }
        return SUCCESS_CODE; // File opened successfully
    }

    size_t syncFile(int* fd, const char* file_path) {
        if (checkFileValidity(*fd) != SUCCESS_CODE) {
            if (openFile(fd, file_path) != SUCCESS_CODE) {
                perror("Failed to open file");
                throw std::runtime_error("Failed to open file");
            } else {
                return SUCCESS_CODE; // File opened successfully
            }
        }
        return SUCCESS_CODE;
    }

    // Fill the buffer with data from the file from the specified offset
    // to the specified size.
    size_t readData(size_t offset, size_t size, void* buffer);

    char operator*();

    size_t operator++();
    size_t operator--();

    size_t operator+=(size_t offset);
    size_t operator-=(size_t offset);

    int operator-(ZeroCopyRead& other);
    int operator+(ZeroCopyRead& other);

    int operator*(ZeroCopyRead& other);
    int operator/(ZeroCopyRead& other);

    size_t getCurrentPosition() const;
    size_t getFileSize() const;
    void resetIterator();
};

#endif // ZERO_COPY_READ_LIBRARY_H
