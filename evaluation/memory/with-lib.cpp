// zero_copy_iter_mem_test.cpp
// Uses ZeroCopyRead::current() + next() to touch each byte in the mmap
// without making an extra copy.

#include "zero-copy-read-library.h"
#include <sys/resource.h>
#include <sys/stat.h>
#include <iostream>
#include <cstdlib>

struct timespec start, end;

unsigned long long calculate_nsec_difference(struct timespec start, struct timespec end) {
/*
 * :param: start: the start time
 * :param: end: the end time
 * :return: the difference between the two times in nanoseconds
*/
    long long nsec_diff = end.tv_nsec - start.tv_nsec;
    long long sec_diff = end.tv_sec - start.tv_sec;
    return sec_diff * 1000000000LL + nsec_diff;
}

long getPeakRSSKB() {
    struct rusage u{};
    getrusage(RUSAGE_SELF, &u);
    return u.ru_maxrss;  // kilobytes
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0]
                  << " <data_1_file> <data_2_file> <lock_file>\n";
        return EXIT_FAILURE;
    }
    const char* data1Path = argv[1];
    const char* data2Path = argv[2];
    const char* lockPath = argv[3];

    try {
        ZeroCopyRead reader1(data1Path, lockPath);
        ZeroCopyRead reader2(data2Path, lockPath);

        // Touch every byte via the zero-copy iterator
        reader1.resetIterator();
        reader2.resetIterator();
        
        int total_sum = 0;

        std::cout << "[before-compute] peak RSS = "
            << getPeakRSSKB() << " KB\n";

        clock_gettime(CLOCK_MONOTONIC, &start);

        while(!(++reader1)&&!(++reader2)) {
            total_sum += (reader1 + reader2);
        }

        clock_gettime(CLOCK_MONOTONIC, &end);

        std::cout << "[zero-copy] peak RSS = "
                  << getPeakRSSKB() << " KB\n";
        std::cout << "[zero-copy] Total sum: " << total_sum << "\n";
        std::cout << "[zero-copy] Time taken: "
                  << calculate_nsec_difference(start, end) << " ns\n";

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
