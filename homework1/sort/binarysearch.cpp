#include <iostream>
#include <vector>

int binarysearch(std::vector<int>& arr, int result){
    int start=0;
    int end=arr.size()-1;

    while(start<=end){
        int middle=start+(end-start)/2;

        if(arr[middle]<result)
            start=middle+1;
        else if(arr[middle]>result)
            end=middle-1;
        else
            return middle;
    }
    return -1;
}

int main(){
    std::vector<int> arr={-4, -2, 1, 14, 67, 88, 105};
    int result=88;
    std::cout<<"The result:"<<binarysearch(arr, result);
    return 0;
}