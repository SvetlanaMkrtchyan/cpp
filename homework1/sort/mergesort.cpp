#include <iostream>
#include <vector>

void merge(std::vector<int>& arr, int start, int middle, int end) {
    int i=start;         
    int j=middle+1;      
    std::vector<int> temp; 
    temp.reserve(end-start+1);

    while (i<=middle && j<=end) {
        if (arr[i]<=arr[j]) 
            temp.push_back(arr[i++]);
        else                  
            temp.push_back(arr[j++]);
    }

    while (i<=middle) 
        temp.push_back(arr[i++]);
    while (j<=end)    
        temp.push_back(arr[j++]);

    for (int k=0; k<(int)temp.size(); ++k)
        arr[start+k]=temp[k];
}

void mergesort(std::vector<int>& arr, int start, int end) {
    if (start<end) {
        int middle=start+(end-start)/2;
        mergesort(arr, start, middle);
        mergesort(arr, middle+1, end);
        merge(arr, start, middle, end);
    }
}

void print(const std::vector<int>& arr) {
    std::cout<<"The result:\n";
    for (size_t i=0; i<arr.size(); ++i) 
        std::cout<<arr[i]<<"\n";
}

int main() {
    std::vector<int> arr={38, 97, 23, -98, 0, 21, 4, 5, 2};
    mergesort(arr, 0, arr.size()-1);
    print(arr);
    return 0;
}