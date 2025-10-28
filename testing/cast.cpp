#include <iostream>
int main(){
    //int x=5;
    //double y=static_cast<double>(x);

    //int k=0x5ffe4c;
    //char* y=reinterpret_cast<char*>(&k);

    //const int m=78;
    //int n=const_cast<int&>(m);

    const char* msg="Lanchka";
    msg="Svetlana";
    char* msg1=const_cast<char*>(msg);
    std::cout<<msg1<<"\n";
    return 0;
}