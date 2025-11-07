#include <iostream>
#include <ctime>

int main(){
    long long a=0;
    clock_t start1=clock();
    for (size_t i=0; i<100000000; ++i) 
        a+=i;
    
    clock_t end1=clock();
    double time1=double(end1-start1)/CLOCKS_PER_SEC;
    std::cout<<"CPU: "<<time1<<" seconds\n";

    const int size=1000000;
    int* x=new int[size];
    for (size_t i=0; i<size; ++i) 
        x[i]=i;

    clock_t start2=clock();
    long long sum=0;
    for (size_t i=0; i<size; ++i) {
        sum+=x[i];
    }
    clock_t end2=clock();
    double time2=double(end2-start2)/CLOCKS_PER_SEC;
    std::cout<<"RAM: " <<time2<<" seconds\n";

    delete[] x;
    x=nullptr;
    return 0;
}