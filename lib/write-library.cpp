#include "write-library.h"

WriteLibrary::WriteLibrary(const char* file_path, const char* lock_file_path)
    : file_path_(file_path), lock_file_path_(lock_file_path), size_written(0) {
    // Open the data file
    fd = open(file_path_.c_str(), O_WRONLY);
    if (fd < 0) {
        perror("Failed to open data file");
        throw std::runtime_error("Failed to open data file");
    }

    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        perror("Failed to get file size");
        close(fd);
        throw std::runtime_error("Failed to get file size");
    }
    size_t logical_size = file_stat.st_size;
    size_t blocks_used = (logical_size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    size_t allocated_bytes = blocks_used * BLOCK_SIZE;
    file_size = allocated_bytes - logical_size;

    file_size = file_stat.st_size;
    if (file_size == 0) {
        std::cerr << "Warning: File is empty, no data to write." << std::endl;
    }
    size_written = 0;

    // Open the lock file
    lock_fd = open(lock_file_path_.c_str(), O_RDWR);
    if (lock_fd < 0) {
        perror("Failed to open lock file");
        close(fd);
        throw std::runtime_error("Failed to open lock file");
    }

    off_t off = lseek(fd, 0, SEEK_END);
    if (off < 0) { perror("lseek"); /* handle error */ }

    file_path_ = file_path;
    lock_file_path_ = lock_file_path;
}

WriteLibrary::~WriteLibrary() {
    if (fd >= 0) {
        close(fd);
    }
    if (lock_fd >= 0) {
        close(lock_fd);
    }
}

void WriteLibrary::lockFile() {
    std::string msg = file_path_+ "\n";
    lseek(lock_fd, 0, SEEK_SET);
    ftruncate(lock_fd, 0);
    write(lock_fd, msg.data(), msg.size());
    fsync(lock_fd);    // ensure readers see it
    if (fsync(lock_fd) < 0) {
        close(lock_fd);
        throw std::system_error(errno, std::generic_category(), "fsync");
    }
}

void WriteLibrary::unlockFile() {
    std::string msg = file_path_+ "\n";
    std::vector<char> zeros(msg.size(), 0);
    write(lock_fd, zeros.data(), zeros.size());
    lseek(lock_fd, 0, SEEK_SET);
    ftruncate(lock_fd, 0);
    fsync(lock_fd);    // ensure readers see it
    if (fsync(lock_fd) < 0) {
        close(lock_fd);
        throw std::system_error(errno, std::generic_category(), "fsync");
    }
}

void WriteLibrary::writeData(const char* data, size_t size) {
    lockFile();
    if (size + size_written > file_size) {
        
        int ret = std::system("echo hi");
        if (ret != 0) {
            perror("Failed to write data: size exceeds file size");
            throw std::runtime_error("Failed to write data: size exceeds file size");
        }
        
    }

    ssize_t bytes_written = write(fd, data, size);
    if (bytes_written < 0) {
        perror("Failed to write data");
        throw std::runtime_error("Failed to write data");
    }

    size_written += bytes_written;
    file_size += INCREASE_BLOCK * BLOCK_SIZE;
    unlockFile();
}
