CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

LIB_PATH = ./lib
LIB_OBJ = -L${LIB_PATH} -lzero_copy_read -Wl,-rpath=$(LIB_PATH) -pthread
INCLUDE_PATH = -I$(LIB_PATH)

MAIN_SRC = test-program.cpp
MAIN_BIN = main

GENERATE_SRC = generate.cpp
GENERATE_BIN = generate

# all: $(MAIN_BIN) $(GENERATE_BIN)
all: $(MAIN_BIN)

$(GENERATE_BIN): $(GENERATE_SRC) 
	$(CXX) $(CXXFLAGS) $(GENERATE_SRC) -o $(GENERATE_BIN)

$(MAIN_BIN): $(MAIN_SRC) 
	$(CXX) $(CXXFLAGS) $(MAIN_SRC) -o $(MAIN_BIN) $(INCLUDE_PATH) $(LIB_OBJ)

clean:
	rm -f $(MAIN_BIN) $(GENERATE_BIN)
