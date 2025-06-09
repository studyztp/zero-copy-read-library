#include "zero-copy-read-library.h"

ZeroCopyRead::ZeroCopyRead(int fp) : fp(fp), current_position(0) {
    struct stat file_stat;
    if (fstat(fp, &file_stat) == -1) {
        perror("fstat failed");
        throw std::runtime_error("Failed to get file size");
    }
    file_size = file_stat.st_size;

    if (file_size == 0) {
        throw std::runtime_error("Cannot mmap empty file");
    }

    base_mmap_ptr = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fp, 0);
    if (base_mmap_ptr == MAP_FAILED) {
        perror("mmap failed");
        throw std::runtime_error("Failed to mmap file");
    }
    iter_mmap_ptr = static_cast<char*>(base_mmap_ptr);
}

ZeroCopyRead::~ZeroCopyRead() {
    if (base_mmap_ptr != MAP_FAILED) {
        munmap(base_mmap_ptr, file_size);
    }
    close(fp);
}

size_t ZeroCopyRead::readData(size_t offset, size_t size, void* buffer) {
    if (offset + size > file_size) {
        return 0; // Out of bounds
    }
    memcpy(buffer, static_cast<char*>(base_mmap_ptr) + offset, size);
    return size;
}

