// test_zero_copy_read.cpp
#include "zero-copy-read-library.h"

#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstdint>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/mman.h>

namespace fs = std::filesystem;

void prepare_lockfile(const std::string& lock_path) {
    const size_t LOCK_SIZE = sizeof(uint64_t) + MAX_BUFFER_SIZE;

    // Open (or create) and resize
    int fd = open(lock_path.c_str(), O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        perror("open lockfile");
        std::exit(1);
    }
    if (ftruncate(fd, LOCK_SIZE) == -1) {
        perror("ftruncate lockfile");
        std::exit(1);
    }

    // mmap the whole thing
    void* ptr = mmap(nullptr, LOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap lockfile");
        std::exit(1);
    }

    // Initialize version = 0
    auto version_ptr = reinterpret_cast<std::atomic<uint64_t>*>(ptr);
    version_ptr->store(0, std::memory_order_release);

    // Write a dummy string that does NOT match our test file path
    char* str_buf = reinterpret_cast<char*>(ptr) + sizeof(uint64_t);
    const char* dummy = "UNLOCKED\n";
    std::strncpy(str_buf, dummy, MAX_BUFFER_SIZE - 1);

    munmap(ptr, LOCK_SIZE);
    close(fd);
}

int main() {
    const std::string test_file = "test.txt";
    const std::string lock_file = "lockfile.lock";

    // 1) Create test.txt with known content
    {
        std::ofstream ofs(test_file);
        ofs << "0123456789\n"
               "ABCDEFGHIJ\n"
               "12345\n";
        // size = 11 + 11 + 6 = 28 bytes
    }

    // 2) Prepare lock file for ZeroCopyRead
    prepare_lockfile(lock_file);

    try {
        // 3) Instantiate two readers
        ZeroCopyRead reader1(test_file.c_str());
        ZeroCopyRead reader2(test_file.c_str());

        // --- File size ---
        std::cout << "File size: " << reader1.getFileSize() << " bytes\n\n";

        // --- readData ---
        char buf[16] = {};
        size_t n = reader1.readData(0, 10, buf);
        std::cout << "readData(0,10): read " << n << " chars -> \"" << buf << "\"\n\n";

        // --- operator* and ++ ---
        std::cout << "Iterating with operator*()/operator++():\n";
        for (int i = 0; i < 12; ++i) {
            char c = *reader1;
            std::cout << c;
            if (reader1.operator++() != SUCCESS_CODE) break;
        }
        std::cout << "\n\n";

        // --- operator+= and operator-= ---
        reader1.resetIterator();
        reader1 += 5;
        std::cout << "After reader1 += 5, *iter = '" << *reader1 << "' (should be '5')\n";
        reader1 -= 3;
        std::cout << "After reader1 -= 3, *iter = '" << *reader1 << "' (should be '2')\n\n";

        // --- Arithmetic operators between reader1 & reader2 ---
        // Move reader2 to position 2 and reader1 to position 5
        reader1.resetIterator(); reader1 += 5;
        reader2.resetIterator(); reader2 += 2;

        // Interpret the bytes at these positions as ints and perform +, -, *, /
        int sum = reader1 + reader2;
        int diff = reader1 - reader2;
        int prod = reader1 * reader2;
        int quot = reader1 / reader2;

        std::cout << "Pos 5 char '" << *reader1 << "' (ascii " << int(*reader1) << ")\n";
        std::cout << "Pos 2 char '" << *reader2 << "' (ascii " << int(*reader2) << ")\n\n";

        std::cout << "reader1 + reader2 = " << sum << "\n";
        std::cout << "reader1 - reader2 = " << diff << "\n";
        std::cout << "reader1 * reader2 = " << prod << "\n";
        std::cout << "reader1 / reader2 = " << quot << "\n";
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
