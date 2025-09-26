#include <bits/stdc++.h>

void minimumBribes(std::vector<int> q) {
    int count=0, n=(int)q.size();
    for (int i=0; i<n; ++i) {
        if (q[i]-(i+1)>2) { 
            std::cout<<"Too chaotic\n"; 
            return; 
        }
    }
    for (int i=0; i<n; ++i) {
        for (int j=std::max(0, q[i]-2); j<i; ++j) {
            if (q[j]>q[i]) 
                count++;
        }
    }
    std::cout<<count<<"\n";
}

int main() {
    std::vector<int> q1 = {2, 1, 5, 3, 4};  
    std::vector<int> q2 = {2, 5, 1, 3, 4};  
    minimumBribes(q1);
    minimumBribes(q2);

    return 0;
}