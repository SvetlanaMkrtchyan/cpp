#include <iostream>

int main() {
    int n=10;
    int x[10]={1, 3, 5, 2, 3, 2, 1, 6, 6, 4};
    std::cout<<"Unique elements:";
    for(int i=0; i<n; ++i){
        bool found=false;
        for(int j=0; j<n; ++j)
            if(i!=j && x[i]==x[j]){
                found=true;
                break;
            }
        if(!found)
            std::cout<<x[i]<<" ";
    } 
    return 0;
}
