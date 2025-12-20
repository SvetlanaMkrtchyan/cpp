#include <iostream>

struct align{
    double a;
    short b;
    int c;
    bool d;
};

int main(){
    align ob;
    std::cout<<sizeof(align)<<std::endl;
    std::cout<<&ob.a<<" "<<&ob.b<<" "<<&ob.c<<" "<<&ob.c<<std::endl;
    std::cout<<"a:"<<(char*)&ob.a-(char*)&ob<<std::endl;
    std::cout<<"b:"<<(char*)&ob.b-(char*)&ob<<std::endl;
    std::cout<<"c:"<<(char*)&ob.c-(char*)&ob<<std::endl;
    std::cout<<"d:"<<(char*)&ob.d-(char*)&ob<<std::endl;
    return 0;
}