#include <iostream>

class Base{
    private:
        int data;
    public:
        Base(int d=0) : data(d) {}
        ~Base(){}

        Base& operator++() {
            ++data;
            return *this; 
        }

        Base operator++(int) {
            Base temp=*this;     
            ++data;                   
            return temp;               
        }

        int get() const { 
            return data; 
        }
};

int main() {
    Base a(4);

    std::cout<<"a="<<a.get()<<std::endl;     

    ++a;                             
    std::cout<<"++a="<<a.get()<<std::endl;     

    a++;                                  
    std::cout<<"a++="<<a.get()<<std::endl;   

    Base b=++a;     
    std::cout<<"++a: a="<<a.get()<<", b="<<b.get()<<std::endl;

    Base c=a++;     
    std::cout<<"a++: a="<<a.get()<<", c="<<c.get()<<std::endl;

    return 0;
}