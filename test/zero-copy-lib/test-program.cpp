// test_zero_copy.cpp

#include "zero-copy-read-library.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <data_file> <lock_file>\n";
        return 1;
    }
    const char* data_path = argv[1];
    const char* lock_path = argv[2];

    std::cout << "Using:\n"
              << "  data file: " << data_path << "\n"
              << "  lock file: " << lock_path << "\n\n";


    try {
        // 3) Instantiate the reader
        ZeroCopyRead reader(data_path, lock_path);

        // -- Test getFileSize()
        size_t sz = reader.getFileSize();
        std::cout << "File size = " << sz << " bytes\n\n";

        // -- Test readData()
        {
            std::vector<char> buf(sz+1);
            size_t n = reader.readData(0, sz, buf.data());
            buf[n] = '\0';
            std::cout << "[readData] full contents:\n"
                      << buf.data() << "\n\n";
        }

        // -- Test atomicReadLine()
        // First, write a line into the lock file so atomicReadLine returns it
        {
            std::ofstream lofs(lock_path, std::ios::trunc);
            lofs << "LOCKWORD\n";  
        }
        {
            char line[MAX_BUFFER_SIZE];
            size_t len = reader.atomicReadLine(line);
            line[len] = '\0';
            std::cout << "[atomicReadLine] got: \"" << line << "\" (len=" << len << ")\n\n";
        }

        // -- Test basic iterator: operator* and operator++()
        {
            reader.resetIterator();
            std::cout << "[iter++] First 5 characters: ";
            for (size_t i = 0; i < 5 && reader.getCurrentPosition() < sz; ++i) {
                char c = *reader;        // operator*()
                std::cout << c;
                ++reader;                // operator++()
            }
            std::cout << "\n\n";
        }

        // -- Test operator--()
        {
            std::cout << "[--iter] ";
            // move back two positions
            --reader;
            --reader;
            std::cout << *reader << "\n\n";
        }

        // -- Test operator+=(n)
        {
            reader.resetIterator();
            reader += 6;  // skip "HelloZ"
            std::cout << "[+6] at pos 6: '" << *reader << "'\n\n";
        }

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    std::cout << "All tests complete.\n";
    return 0;
}
