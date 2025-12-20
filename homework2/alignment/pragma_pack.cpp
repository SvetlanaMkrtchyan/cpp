#include <iostream>

struct Before {
    char a;  
    int b;     
    long long c;     
};

#pragma pack(push, 1)
struct After {
    char a;      
    int b;      
    long long c;      
};
#pragma pack(pop)

int main() {
    std::cout<<"sizeof(Before): "<<sizeof(Before);
    std::cout<<"\nsizeof(After): "<<sizeof(After);
    
    After p;
    std::cout<<"\nAfter:\n";
    std::cout<<"&p.a: "<<(void*)&p.a<<std::endl;
    std::cout<<"&p.b: "<<(void*)&p.b<<std::endl;
    std::cout<<"&p.c: "<<(void*)&p.c<<std::endl;
    
    return 0;
}
