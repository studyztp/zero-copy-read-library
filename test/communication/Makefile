CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

LIB_PATH = ../../lib
LIB_OBJ = -L${LIB_PATH} -lzero_copy_read -lwrite -Wl,-rpath=$(LIB_PATH) -pthread
INCLUDE_PATH = -I$(LIB_PATH)

MAIN_SRC = test-program.cpp
MAIN_BIN = main

all: $(MAIN_BIN)

$(MAIN_BIN): $(MAIN_SRC) 
	$(CXX) $(CXXFLAGS) $(MAIN_SRC) -o $(MAIN_BIN) $(INCLUDE_PATH) $(LIB_OBJ)

clean:
	rm -f $(MAIN_BIN) 
