#include <bits/stdc++.h>

int minimumSwaps(std::vector<int> arr) {
    int n=(int)arr.size();
    int count=0;
    for (int i=0; i<n; ++i) {
        while (arr[i]!=i+1) {
            int correctIndex=arr[i]-1;
            std::swap(arr[i], arr[correctIndex]);
            count++;
        }
    }
    return count;
}

int main() {
    std::vector<int> a1={4, 3, 1, 2};    
    std::vector<int> a2={2, 3, 4, 1, 5};  
    std::vector<int> a3={1, 3, 5, 2, 4, 6, 7};

    std::cout<<"The results:";
    std::cout<<minimumSwaps(a1)<<"\n";
    std::cout<<minimumSwaps(a2)<<"\n";
    std::cout<<minimumSwaps(a3)<<"\n";
    return 0;
}