// test_write.cpp

#include "write-library.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <cerrno>
#include <cstring>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <data_file> <lock_file>\n";
        return 1;
    }
    const char* data_path = argv[1];
    const char* lock_path = argv[2];

    // 1) Prepare data file: truncate/create and grow to BLOCK_SIZE
    {
        int df = open(data_path, O_CREAT | O_RDWR, 0666);
        if (df < 0) {
            std::cerr << "open(" << data_path << "): " << std::strerror(errno) << "\n";
            return 1;
        }
        if (ftruncate(df, BLOCK_SIZE) != 0) {
            std::cerr << "ftruncate(data): " << std::strerror(errno) << "\n";
            close(df);
            return 1;
        }
        close(df);
    }

    // 2) Prepare lock file: truncate/create to 1 byte
    {
        int lf = open(lock_path, O_CREAT | O_RDWR, 0666);
        if (lf < 0) {
            std::cerr << "open(" << lock_path << "): " << std::strerror(errno) << "\n";
            return 1;
        }
        if (ftruncate(lf, 1) != 0) {
            std::cerr << "ftruncate(lock): " << std::strerror(errno) << "\n";
            close(lf);
            return 1;
        }
        close(lf);
    }

    // 3) Write a few messages
    try {
        WriteLibrary writer(data_path, lock_path);
        std::vector<std::string> messages = {
            "Hello, world!\n",
            "The quick brown fox jumps over the lazy dog.\n",
            "Final line of test.\n"
        };

        for (const auto& msg : messages) {
            std::cout << "[Test] Writing: " << msg;
            writer.writeData(msg.c_str(), msg.size());
        }
    } catch (const std::exception& ex) {
        std::cerr << "WriteLibrary error: " << ex.what() << "\n";
        return 1;
    }

    // 4) Read back and print the file contents
    {
        std::ifstream ifs(data_path);
        if (!ifs) {
            std::cerr << "Failed to open " << data_path << " for reading\n";
            return 1;
        }
        std::cout << "\n[Test] Final file contents:\n";
        std::string line;
        while (std::getline(ifs, line)) {
            std::cout << line << "\n";
        }
    }

    return 0;
}
