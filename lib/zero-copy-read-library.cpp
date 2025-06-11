#include "zero-copy-read-library.h"

ZeroCopyRead::ZeroCopyRead(const char* file_path, const char* lock_file_path) : current_position(0), ready(false) {
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
    lock_file_path_ = lock_file_path;

    lock_fd = open(lock_file_path_.c_str(), O_RDWR);
    if (lock_fd == -1) {
        perror("Failed to open lock file");
        throw std::runtime_error("Failed to open lock file");
    }

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

size_t ZeroCopyRead::atomicReadLine(char* buffer) {
    struct stat file_stat;
    if (fstat(lock_fd, &file_stat) == -1) {
        perror("fstat failed");
        throw std::runtime_error("Failed to get lock file size");
    }
    size_t lock_file_file_size = file_stat.st_size;

    if (lock_file_file_size == 0) {
        return 0; // No data to read
    }

    lock_mmap_ptr = mmap(nullptr, lock_file_file_size, PROT_READ | PROT_WRITE, MAP_SHARED, lock_fd, 0);
    size_t char_to_read = lock_file_file_size / sizeof(char);

    char* str_ptr = static_cast<char*>(lock_mmap_ptr);

    size_t i = 0;
    while (i < char_to_read) {
        char c = str_ptr[i];
        if (c == '\n') {
            break;
        } else if (c == '\0') {
            break;
        }
        buffer[i] = c;
        i++;
    }

    return i;

}

void ZeroCopyRead::readLockfile() {
    char buffer[MAX_BUFFER_SIZE];  // Make sure this is large enough
    if (atomicReadLine(buffer) == 0) {
        return; // No data to read
    }
    std::string read_buffer(buffer);
    while (std::strcmp(read_buffer.c_str(), file_path_.c_str()) == 0) {
        // Wait for the lock to be released
        syncFile(&lock_fd, lock_file_path_.c_str());
        memset(buffer, 0, sizeof(buffer)); // Clear the buffer
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (atomicReadLine(buffer) == 0) {
            break; // Exit if no data is read
        }
        std::string read_buffer(buffer);
    }
    // Lock is released, we can proceed
}

size_t ZeroCopyRead::readData(size_t offset, size_t size, void* buffer) {
    if (offset + size > file_size) {
        return 0; // Out of bounds
    }

    readLockfile();
    syncFile(&fd, file_path_.c_str());
    
    memcpy(buffer, static_cast<char*>(base_mmap_ptr) + offset, size);
    return size;
}

char ZeroCopyRead::operator*(){
    //  can not be a constant operator*() because it needs to modify the 
    // fd if the file is not valid or needs to be synced
    readLockfile();
    syncFile(&fd, file_path_.c_str());
    
    return *iter_mmap_ptr;
}

size_t ZeroCopyRead::operator++() {
    if (current_position + 1 >= file_size) {
        return ERROR_CODE; // End of file reached
    }
    
    readLockfile();
    syncFile(&fd, file_path_.c_str());
    
    iter_mmap_ptr++;
    current_position++;
    return SUCCESS_CODE; // Successfully moved to the next character
}

size_t ZeroCopyRead::operator--() {
    if (current_position == 0) {
        return ERROR_CODE; // Cannot move back, already at the start
    }

    readLockfile();
    syncFile(&fd, file_path_.c_str());
    
    iter_mmap_ptr--;
    current_position--;
    return SUCCESS_CODE; // Successfully moved to the previous character
}

size_t ZeroCopyRead::operator+=(size_t offset) {
    if (current_position + offset >= file_size) {
        return ERROR_CODE; // Out of bounds
    }
    readLockfile();
    syncFile(&fd, file_path_.c_str());

    iter_mmap_ptr += offset;
    current_position += offset;
    return SUCCESS_CODE; // Successfully moved forward by offset
}

size_t ZeroCopyRead::operator-=(size_t offset) {
    if (current_position < offset) {
        return ERROR_CODE; // Out of bounds
    }

    readLockfile();
    syncFile(&fd, file_path_.c_str());
    
    iter_mmap_ptr -= offset;
    current_position -= offset;
    return SUCCESS_CODE; // Successfully moved backward by offset
}

int ZeroCopyRead::operator-(ZeroCopyRead& other) {
    readLockfile();
    syncFile(&fd, file_path_.c_str());
    other.readLockfile();
    other.syncFile(&other.fd, other.file_path_.c_str());
    
    return *reinterpret_cast<int*>(iter_mmap_ptr) - *reinterpret_cast<int*>(other.iter_mmap_ptr);
}

int ZeroCopyRead::operator+(ZeroCopyRead& other) {
    readLockfile();
    syncFile(&fd, file_path_.c_str());
    other.readLockfile();
    other.syncFile(&other.fd, other.file_path_.c_str());
    return *reinterpret_cast<int*>(iter_mmap_ptr) + *reinterpret_cast<int*>(other.iter_mmap_ptr);
}

int ZeroCopyRead::operator*(ZeroCopyRead& other) {
    readLockfile();
    syncFile(&fd, file_path_.c_str());
    other.readLockfile();
    other.syncFile(&other.fd, other.file_path_.c_str());
    return *reinterpret_cast<int*>(iter_mmap_ptr) * *reinterpret_cast<int*>(other.iter_mmap_ptr);
}

int ZeroCopyRead::operator/( ZeroCopyRead& other) {
    readLockfile();
    syncFile(&fd, file_path_.c_str());
    other.readLockfile();
    other.syncFile(&other.fd, other.file_path_.c_str());
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



