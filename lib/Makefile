# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -fPIC -shared 
LIB = -pthread

# Targets
STATIC_LIB = libzero_copy_read.a libwrite.a
SHARED_LIB = libzero_copy_read.so libwrite.so
OBJS = zero-copy-read-library.o write-library.o
HEADERS = zero-copy-read-library.h write-library.h

# Default target: build everything
all: $(STATIC_LIB) $(SHARED_LIB)

# Build object file
zero-copy-read-library.o: zero-copy-read-library.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c zero-copy-read-library.cpp -o $@ $(LIB)

write-library.o: write-library.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c write-library.cpp -o $@ $(LIB)

# Static library
$(STATIC_LIB): $(OBJS)
	ar rcs $@ $^

# Shared library
$(SHARED_LIB): $(OBJS)
	$(CXX) -shared -o $@ $^

# Clean build artifacts
clean:
	rm -f *.o *.a *.so 

.PHONY: all clean
