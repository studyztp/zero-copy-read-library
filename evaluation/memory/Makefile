CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

LIB_PATH = ../../lib
LIB_OBJ = -L${LIB_PATH} -lzero_copy_read -lwrite -Wl,-rpath=$(LIB_PATH) -pthread
INCLUDE_PATH = -I$(LIB_PATH)

WITH_LIB = with_lib
WITHOUT_LIB = without_lib
WITH_LIB_SRC = with-lib.cpp
WITHOUT_LIB_SRC = without-lib.cpp

all: $(WITH_LIB) $(WITHOUT_LIB)

$(WITH_LIB): $(WITH_LIB_SRC)
	$(CXX) $(CXXFLAGS) $(WITH_LIB_SRC) -o $(WITH_LIB) $(INCLUDE_PATH) $(LIB_OBJ)
$(WITHOUT_LIB): $(WITHOUT_LIB_SRC)
	$(CXX) $(CXXFLAGS) $(WITHOUT_LIB_SRC) -o $(WITHOUT_LIB) $(INCLUDE_PATH)

clean:
	rm -f $(WITH_LIB) $(WITHOUT_LIB)
