// concurrency_test.cpp
// Combines writer and reader using multi-threading to test zero-copy-read-library and write-library working together

#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <cstring>

#include "write-library.h"
#include "zero-copy-read-library.h"

// Number of iterations for writer and reader loops
static const int NUM_ITERATIONS = 5;
// Delay between operations in milliseconds
static const int WRITE_DELAY_MS = 1000;
static const int READ_DELAY_MS = 1200;

void writerTask(const char* data_file, const char* lock_file) {
    try {
        WriteLibrary writer(data_file, lock_file);
        for (int i = 1; i <= NUM_ITERATIONS; ++i) {
            std::string msg = "[Writer] Message " + std::to_string(i) + "\n";
            std::cout << msg;
            writer.writeData(msg.c_str(), msg.size());
            std::this_thread::sleep_for(std::chrono::milliseconds(WRITE_DELAY_MS));
        }
    } catch (const std::exception& e) {
        std::cerr << "Writer error: " << e.what() << std::endl;
    }
}

void readerTask(const char* data_file, const char* lock_file) {
    try {
        ZeroCopyRead reader(data_file, lock_file);
        char character;
        for (int i = 1; i <= NUM_ITERATIONS; ++i) {
            // Read full file content
            character = *reader;
            if (++reader) {
                std::cerr << "Error reading character at iteration " << i << std::endl;
                continue; // Skip to next iteration if read fails
            }
            std::cout << "[Reader] Iteration " << i << ", bytesRead=" << character << "\n";
            std::cout << std::endl;

            std::this_thread::sleep_for(std::chrono::milliseconds(READ_DELAY_MS));
        }
    } catch (const std::exception& e) {
        std::cerr << "Reader error: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <data_file> <lock_file>\n";
        return 1;
    }
    const char* data_file = argv[1];
    const char* lock_file = argv[2];

    std::cout << "Starting concurrency test on:\n"
              << "  data file: " << data_file << "\n"
              << "  lock file: " << lock_file << "\n"
              << "  iterations: " << NUM_ITERATIONS << "\n\n";

    // Launch writer and reader in separate threads
    std::thread writerThread(writerTask, data_file, lock_file);
    std::thread readerThread(readerTask, data_file, lock_file);

    // Wait for both to finish
    writerThread.join();
    readerThread.join();

    std::cout << "Concurrency test complete." << std::endl;
    return 0;
}
