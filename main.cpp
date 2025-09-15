#include <iostream>

int main() {
    int n=10;
    int x[10]={1, 3, 5, 2, 3, 2, 1, 6, 6, 4};
    
    int temp=0;
    for(int i=0; i<n; ++i)
        temp^=x[i];
    
    int result=temp & -temp;
    int a=0, b=0;

    for(int i=0; i<n; ++i){
        if(result & x[i])
            a^=x[i];
        else 
            b^=x[i];
    }
    std::cout<<"Unique elements:"<<a<<" "<<b; 
    return 0;
}