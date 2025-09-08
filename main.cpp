#include <iostream>

int main() {
    int n = 7;
    int x[7] = {0, 1, 3, 7, 7, 0, 1};
    int temp=0;
    for (int i = 0; i < n; i++) {
        temp ^= x[i];
    }
    std::cout<<temp;
    return 0;
}