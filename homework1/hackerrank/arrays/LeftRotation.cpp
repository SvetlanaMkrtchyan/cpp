#include <bits/stdc++.h>

std::vector<int> rotLeft(std::vector<int> a, int d) {
    int n=(int)a.size();
    if (n==0) 
        return a;
    d%=n;
    std::vector<int> x(n);
    for (size_t i=0; i<n; ++i) {
        x[i]=a[(i+d)%n];
    }
    return x;
}

int main() {
    std::vector<int> a = {1, 2, 3, 4, 5};
    int d = 2;

    std::vector<int> result = rotLeft(a, d);

    std::cout<<"The result:";
    for (size_t i=0; i<result.size(); ++i) {
        std::cout<<result[i];
        if (i+1<result.size()) 
            std::cout<<" ";
    }
    return 0;
}