// plain_read_iter_mem_test.cpp
// Reads two files in 4-byte chunks via read(), sums them, and reports peak RSS.

#include <sys/resource.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <cerrno>
#include <cstring>
#include <time.h> 
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

static constexpr size_t INTS_PER_CHUNK = 20;
long getPeakRSSKB() {
    struct rusage u{};
    getrusage(RUSAGE_SELF, &u);
    return u.ru_maxrss;  // kilobytes
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <data_1_file> <data_2_file>\n";
        return EXIT_FAILURE;
    }
    const char* path1 = argv[1];
    const char* path2 = argv[2];
    // argv[3] is the lock file, but we don't use it here

    // Open both files read‐only
    int fd1 = open(path1, O_RDONLY);
    if (fd1 < 0) {
        std::cerr << "open(" << path1 << "): " << std::strerror(errno) << "\n";
        return EXIT_FAILURE;
    }
    int fd2 = open(path2, O_RDONLY);
    if (fd2 < 0) {
        std::cerr << "open(" << path2 << "): " << std::strerror(errno) << "\n";
        close(fd1);
        return EXIT_FAILURE;
    }


    int64_t total_sum = 0;
    int32_t buf1[INTS_PER_CHUNK];
    int32_t buf2[INTS_PER_CHUNK];

    std::cout << "[before-compute] peak RSS = "
              << getPeakRSSKB() << " KB\n";
    clock_gettime(CLOCK_MONOTONIC, &start);

    while (true) {
        // Attempt to read INTS_PER_CHUNK ints from each file
        ssize_t bytes1 = read(fd1, buf1, sizeof(buf1));
        ssize_t bytes2 = read(fd2, buf2, sizeof(buf2));

        // If we couldn't read a full chunk from either, break
        if (bytes1 <= 0 || bytes2 <= 0) {
            break;
        }

        // Determine how many ints were actually read
        size_t ints1 = bytes1 / sizeof(int32_t);
        size_t ints2 = bytes2 / sizeof(int32_t);
        size_t n = std::min(ints1, ints2);

        // Sum pairwise for this chunk
        for (size_t i = 0; i < n; ++i) {
            total_sum += static_cast<int64_t>(buf1[i])
                       + static_cast<int64_t>(buf2[i]);
        }

        // If one file ended mid‐chunk, we stop
        if (ints1 < INTS_PER_CHUNK || ints2 < INTS_PER_CHUNK) {
            break;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    std::cout << "[plain-read] peak RSS = "
              << getPeakRSSKB() << " KB\n";
    std::cout << "[plain-read] Total sum: " << total_sum << "\n";
    std::cout << "[plain-read] Time taken: "
              << calculate_nsec_difference(start, end) << " ns\n";


    close(fd1);
    close(fd2);

    return EXIT_SUCCESS;
}
