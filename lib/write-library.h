#ifndef WRITE_LIBRARY_H
#define WRITE_LIBRARY_H

#include <fcntl.h>    // for open flags
#include <unistd.h>   // for write(), close()
#include <sys/stat.h> // for mode constants
#include <cerrno>     // for errno
#include <cstring>    // for strerror
#include <iostream>
#include <cstdlib>
#include <unistd.h>   // for ftruncate
#include <cstdint> 
#include <sys/mman.h>
#include <vector>

static constexpr std::uint64_t BLOCK_SIZE = 2ULL * 1024 * 1024;
#define INCREASE_BLOCK 1
#define MAX_BUFFER_SIZE 1024

class WriteLibrary {
    private:
        int fd;                        // File descriptor for the data file
        int lock_fd;                // File descriptor for the lock file
        size_t file_size;            // Size of the file
        size_t size_written;
        std::string file_path_;      // Path to the data file
        std::string lock_file_path_; // Path to the lock file

    public:
        // Constructor
        WriteLibrary(const char* file_path, const char* lock_file_path);
        
        // Destructor
        ~WriteLibrary();

        // Method to lock the file for writing
        void lockFile();
        
        // Method to unlock the file after writing
        void unlockFile();
        
        // Method to write data to the file
        void writeData(const char* data, size_t size);

};

#endif // WRITE_LIBRARY_H