#include <iostream>
#include <cstdint>

int main() {
    uint64_t b=1;
    double s=*reinterpret_cast<double*>(&b);
    std::cout<<s;
    return 0;
}