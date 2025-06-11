Got it! Here's the entire README content written cleanly and fully as a `.md` file from top to bottom, suitable for direct use as `README.md` in your repo:


# Zero-Copy Read Library

This repository provides a **zero-copy read library** and a **coordinated write library** designed for efficient file access in disaggregated or tiered memory systems (e.g., DAX, CXL). It enables a *reader* and a *writer* process to communicate through a memory-mapped file and a lockfile, allowing fast read operations without data duplication or page cache interference.

## Features

- ✅ Zero-copy memory-mapped read interface  
- 🔒 Thread-safe coordination using a shared lockfile  
- 🔁 Atomic reader-writer synchronization via file-level signaling  
- ➕ Simple iterator-style API for sequential access  
- ⚙️ Minimal dependencies, intended for system-level experimentation

## Directory Structure

```
├── examples/                     # Example test programs for reading and writing
├── lib/
│   ├── write-library.\*           # Writer-side implementation
│   ├── zero-copy-read-library.\* # Reader-side implementation
├── test\_data/
│   ├── data.txt                  # Example data file
│   └── lockfile.lock             # Example lock file
├── Makefile                      # Build script
```

## Build Instructions

To compile the example and test programs, run:

```bash
make
````

This will produce binaries such as `generate`, `with_lib`, etc.

## Usage

### 1. Writer Program

The writer appends new data to the file and uses the lockfile to signal availability.

```cpp
WriteLibrary writer("data.txt", "lockfile.lock");
writer.append("Hello World\n");
writer.unlockFile();  // Signals that writing is done
```

### 2. Reader Program

The reader reads the latest data directly from the memory-mapped file after consulting the lockfile.

```cpp
int fd = open("data.txt", O_RDONLY);
ZeroCopyRead reader(fd);
std::string line = reader.readLine();  // Reads one line
```

### 3. Combined Reader + Writer (Multithreaded Test)

Run both writer and reader concurrently in a test:

```bash
./with_lib
```

This simulates the interaction between writer and reader using two threads.

## System Requirements

* Linux with support for `mmap()` and DAX (e.g., `/mnt/famfs-mount`)
* GCC with C++17 support
* POSIX-compliant system calls (`open`, `ftruncate`, `fsync`, `flock`)
* Tested in QEMU with tiered memory simulation

## Design Notes

* The lockfile is updated by the writer using `ftruncate` and `write`, and read by the reader using a custom atomic line reader.
* The reader assumes newline-terminated messages (e.g., file paths).
* Files are expected to reside on a filesystem that bypasses the page cache (e.g., DAX/NVDIMM mount points).
* No actual network communication is used; coordination is done through the filesystem.

## License

This project is licensed under the [MIT License](LICENSE).

## Author

**ZT Qiu**
[https://github.com/studyztp](https://github.com/studyztp)

```

Let me know if you'd like this saved as a downloadable file or with CI/build/test badges included!
```
