// generate.cpp
#include <fstream>

int main() {
    std::ofstream out("data.bin", std::ios::binary);
    int nums[] = {10, 2};
    out.write(reinterpret_cast<char*>(nums), sizeof(nums));
    return 0;
}
