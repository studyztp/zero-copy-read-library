// test_lock.cpp

#include "write-library.h"
#include "zero-copy-read-library.h"

#include <atomic>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

int main() {
    const char* dataPath = "data.txt";
    const char* lockPath = "lockfile.lock";

    // 3) Construct writer & reader
    WriteLibrary writer(dataPath, lockPath);
    ZeroCopyRead reader(dataPath, lockPath);

    // Writer: grab lock, sleep, release
    auto writerTask = [&]() {
        std::cout << "[Writer] locking...\n";
        writer.lockFile();
        std::cout << "[Writer] locked, holding for 3s\n";
        std::this_thread::sleep_for(std::chrono::seconds(3));
        writer.unlockFile();
        std::cout << "[Writer] unlocked\n";
    };

    // Reader: attempt to acquire lock
    auto readerTask = [&]() {
        std::cout << "[Reader] waiting on lock...\n";
        auto start = std::chrono::steady_clock::now();
        reader.readLockfile();
        auto elapsed = std::chrono::steady_clock::now() - start;
        auto secs = 
          std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
        std::cout << "[Reader] acquired after " << secs << "s\n";
    };

    std::thread w(writerTask);
    // give writer a tiny head start so it definitely locks before reader starts
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread r(readerTask);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    w.join();
    r.join();

    printf("Second Test\n");

    std::thread w2(writerTask);
    // give writer a tiny head start so it definitely locks before reader starts
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread r2(readerTask);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    w2.join();
    r2.join();

    return 0;
}
