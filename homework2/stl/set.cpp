#include <iostream>
#include <set>

int main() {
    std::set<int> uni_asc={123, 0, -45, 34, 23, 34};

    for (auto it=uni_asc.begin(); it!=uni_asc.end(); ++it) 
        std::cout<<*it<<" "; 
    return 0;
}