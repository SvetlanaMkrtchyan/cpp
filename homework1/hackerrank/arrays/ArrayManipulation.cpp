#include <bits/stdc++.h>

long arrayManipulation(int n, std::vector<std::vector<int>> queries) {
    std::vector<int> x(n+2, 0);
    for (int i=0; i<(int)queries.size(); ++i) {
        int a=queries[i][0];
        int b=queries[i][1];
        int k=queries[i][2];
        x[a]+=(int)k;
        if (b+1<=n) 
            x[b+1]-=(int)k;
    }
    int cur=0, max=0;
    for (int i=1; i<=n; ++i) {
        cur+=x[i];
        if (cur>max) 
            max=cur;
    }
    return (int)max;
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