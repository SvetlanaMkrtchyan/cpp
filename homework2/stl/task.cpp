#include <iostream>
#include <vector>
#include <algorithm>

class Base {
    public:
        static int count;

        bool operator()(int a, int b) {
            count++;
            bool aOdd = (a % 2 != 0);
            bool bOdd = (b % 2 != 0);

            if (aOdd && !bOdd) 
                return true;  
            if (!aOdd && bOdd) 
                return false; 

            return a < b;
        }
};

int Base::count = 0;

int main() {
    std::vector<int> v1={2, 0, 5, 4};
    std::vector<int> v2={7, 1, 6, 3};

    std::vector<int> v=v1;
    v.insert(v.end(), v2.begin(), v2.end());

    sort(v.begin(), v.end(), Base());

    std::cout<<"Sorted array:";
    for (int x : v) 
        std::cout<<x<<" ";
    std::cout<<std::endl;

    std::cout<<Base::count<<std::endl;

    return 0;
}