// main.cpp
#include "zero-copy-read-library.h"
#include <fcntl.h>      // open
#include <iostream>
#include <unistd.h>     // close
#include <stdexcept>    // std::runtime_error

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <file1> <file2>\n";
        return 1;
    }

    const char* file1 = argv[1];
    const char* file2 = argv[2];

    int fd1 = open(file1, O_RDONLY);
    if (fd1 == -1) {
        perror(("Failed to open file: " + std::string(file1)).c_str());
        return 1;
    }

    int fd2 = open(file2, O_RDONLY);
    if (fd2 == -1) {
        perror(("Failed to open file: " + std::string(file2)).c_str());
        close(fd1);
        return 1;
    }

    try {
        ZeroCopyRead zr1(fd1);
        ZeroCopyRead zr2(fd2);

        std::cout << "File 1 int: " << *zr1 << "\n";
        std::cout << "File 2 int: " << *zr2 << "\n";

        std::cout << "Sum: " << (zr1 + zr2) << "\n";
        std::cout << "Difference: " << (zr1 - zr2) << "\n";
        std::cout << "Product: " << (zr1 * zr2) << "\n";
        std::cout << "Quotient: " << (zr1 / zr2) << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        close(fd1);
        close(fd2);
        return 1;
    }

    // Destructors will close the file descriptors
    return 0;
}
