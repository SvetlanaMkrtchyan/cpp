#include <bits/stdc++.h>

long arrayManipulation(int n, std::vector<std::vector<int>> queries) {
    std::vector<long long> diff(n + 2, 0);
    for (int i = 0; i < (int)queries.size(); ++i) {
        int a=queries[i][0];
        int b=queries[i][1];
        int k=queries[i][2];
        diff[a]+=(long long)k;
        if (b+1<=n) 
            diff[b+1]-=(long long)k;
    }
    long long current=0, max=0;
    for (int i=1; i<=n; ++i) {
        current+=diff[i];
        if (current>max) 
            max=current;
    }
    return (long)max;
}

int main() {
    int n=5;
    std::vector<std::vector<int>> queries = {
        {1, 2, 100},
        {2, 5, 100},
        {3, 4, 100}
    };

    long result=arrayManipulation(n, queries);
    std::cout<<"The result:"<<result<<"\n"; 
    return 0;
}