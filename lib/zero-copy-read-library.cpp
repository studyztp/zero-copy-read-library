#include "zero-copy-read-library.h"

ZeroCopyRead::ZeroCopyRead(const char* file_path) : current_position(0), ready(false) {
    struct stat file_stat;
    fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        throw std::runtime_error("Failed to open file");
    }
    if (fstat(fd, &file_stat) == -1) {
        perror("fstat failed");
        throw std::runtime_error("Failed to get file size");
    }
    file_size = file_stat.st_size;
    file_path_ = file_path;
    lock_file_path_ = "lockfile.lock"; // Example lock file path

    lock_fd = open(lock_file_path_.c_str(), O_RDWR);
    if (lock_fd == -1) {
        perror("Failed to open lock file");
        throw std::runtime_error("Failed to open lock file");
    }
    lock_mmap_ptr = mmap(nullptr, sizeof(uint64_t) + MAX_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, lock_fd, 0);

    if (file_size == 0) {
        throw std::runtime_error("Cannot mmap empty file");
    }

    base_mmap_ptr = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
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
    close(fd);
}

void ZeroCopyRead::atomicReadLine(char* buffer) {
    uint64_t version1, version2;
    do {
        // Step 1: read version before
        version1 = reinterpret_cast<std::atomic<uint64_t>*>(lock_mmap_ptr)->load(std::memory_order_acquire);

        // Step 2: if version is odd, writer in progress — retry
        if (version1 % 2 != 0) continue;

        // Step 3: read the string into buffer
        const char* str_ptr = reinterpret_cast<const char*>(reinterpret_cast<const uint8_t*>(lock_mmap_ptr) + sizeof(uint64_t));

        size_t i = 0;
        while (i < sizeof(buffer) - 1) {
            char c = str_ptr[i];
            buffer[i] = c;
            if (c == '\n') {
                buffer[i + 1] = '\0';  // null-terminate
                break;
            } else if (c == '\0') {
                buffer[i] = '\0';  // null-terminate if we hit the end of the string
                break;
            }
            i++;
        }

        // Step 4: read version again
        version2 = reinterpret_cast<std::atomic<uint64_t>*>(lock_mmap_ptr)->load(std::memory_order_acquire);

    } while (version1 != version2 || version1 % 2 != 0);
}

void ZeroCopyRead::readLockfile() {
    syncFile(&lock_fd, lock_file_path_.c_str());
    char buffer[1024];  // Make sure this is large enough
    atomicReadLine(buffer);
    while (std::strcmp(buffer, file_path_.c_str()) == 0) {
        // Wait for the lock to be released
        syncFile(&lock_fd, lock_file_path_.c_str());
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        atomicReadLine(buffer);
    }
    // Lock is released, we can proceed
}

size_t ZeroCopyRead::readData(size_t offset, size_t size, void* buffer) {
    if (offset + size > file_size) {
        return 0; // Out of bounds
    }

    syncFile(&fd, file_path_.c_str());
    readLockfile();
    
    memcpy(buffer, static_cast<char*>(base_mmap_ptr) + offset, size);
    return size;
}

char ZeroCopyRead::operator*(){
    //  can not be a constant operator*() because it needs to modify the 
    // fd if the file is not valid or needs to be synced
    syncFile(&fd, file_path_.c_str());
    readLockfile();
    return *iter_mmap_ptr;
}

size_t ZeroCopyRead::operator++() {
    if (current_position + 1 >= file_size) {
        return ERROR_CODE; // End of file reached
    }
    syncFile(&fd, file_path_.c_str());
    readLockfile();
    iter_mmap_ptr++;
    current_position++;
    return SUCCESS_CODE; // Successfully moved to the next character
}

size_t ZeroCopyRead::operator--() {
    if (current_position == 0) {
        return ERROR_CODE; // Cannot move back, already at the start
    }
    syncFile(&fd, file_path_.c_str());
    readLockfile();
    iter_mmap_ptr--;
    current_position--;
    return SUCCESS_CODE; // Successfully moved to the previous character
}

size_t ZeroCopyRead::operator+=(size_t offset) {
    if (current_position + offset >= file_size) {
        return ERROR_CODE; // Out of bounds
    }
    syncFile(&fd, file_path_.c_str());
    readLockfile();
    iter_mmap_ptr += offset;
    current_position += offset;
    return SUCCESS_CODE; // Successfully moved forward by offset
}

size_t ZeroCopyRead::operator-=(size_t offset) {
    if (current_position < offset) {
        return ERROR_CODE; // Out of bounds
    }
    syncFile(&fd, file_path_.c_str());
    readLockfile();
    iter_mmap_ptr -= offset;
    current_position -= offset;
    return SUCCESS_CODE; // Successfully moved backward by offset
}

int ZeroCopyRead::operator-(ZeroCopyRead& other) {
    syncFile(&fd, file_path_.c_str());
    readLockfile();
    other.syncFile(&other.fd, other.file_path_.c_str());
    other.readLockfile();
    return *reinterpret_cast<int*>(iter_mmap_ptr) - *reinterpret_cast<int*>(other.iter_mmap_ptr);
}

int ZeroCopyRead::operator+(ZeroCopyRead& other) {
    syncFile(&fd, file_path_.c_str());
    readLockfile();
    other.syncFile(&other.fd, other.file_path_.c_str());
    other.readLockfile();
    return *reinterpret_cast<int*>(iter_mmap_ptr) + *reinterpret_cast<int*>(other.iter_mmap_ptr);
}

int ZeroCopyRead::operator*(ZeroCopyRead& other) {
    syncFile(&fd, file_path_.c_str());
    readLockfile();
    other.syncFile(&other.fd, other.file_path_.c_str());
    other.readLockfile();
    return *reinterpret_cast<int*>(iter_mmap_ptr) * *reinterpret_cast<int*>(other.iter_mmap_ptr);
}

int ZeroCopyRead::operator/( ZeroCopyRead& other) {
    syncFile(&fd, file_path_.c_str());
    readLockfile();
    other.syncFile(&other.fd, other.file_path_.c_str());
    other.readLockfile();
    int right_value = *reinterpret_cast<int*>(other.iter_mmap_ptr);
    if (right_value == 0) {
        throw std::runtime_error("Division by zero");
    }
    return *reinterpret_cast<int*>(iter_mmap_ptr) / right_value;
}

size_t ZeroCopyRead::getCurrentPosition() const {
    return current_position;
}
size_t ZeroCopyRead::getFileSize() const {
    return file_size;
}
void ZeroCopyRead::resetIterator() {
    iter_mmap_ptr = static_cast<char*>(base_mmap_ptr);
    current_position = 0;
}



