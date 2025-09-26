#include <bits/stdc++.h>

int hourglassSum(std::vector<std::vector<int>> arr) {
    int temp=INT_MIN;
        for (size_t i=0; i<=3; ++i) {
            for (size_t j=0; j<=3; ++j) {
                int sum=0;
                sum+=arr[i][j]+arr[i][j+1]+arr[i][j+2];
                sum+=arr[i+1][j+1];
                sum+=arr[i+2][j]+arr[i+2][j+1]+arr[i+2][j+2];
                temp=std::max(temp, sum);
            }
        }
        return temp;
}

int main() {
    std::vector<std::vector<int>> arr={
        {1, 1, 1, 0, 0, 0},
        {0, 1, 0, 0, 0, 0},
        {1, 1, 1, 0, 0, 0},
        {0, 0, 2, 4, 4, 0},
        {0, 0, 0, 2, 0, 0},
        {0, 0, 1, 2, 4, 0}
    };

    int result=hourglassSum(arr);
    std::cout<<"The result:"<<result<<"\n";
    return 0;
}