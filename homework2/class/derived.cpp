#include <iostream>

class Base {
    protected:
        int data;
    public:
        Base (int data1) : data(data1){}
        ~Base(){}

        int get(){
            return data*data;
        }
};

class Derived: public Base{
    public:
        Derived(int data2): Base(data2){}
        ~Derived(){}

        bool foo(){
            bool found=false;
            if(get()>36)
                found=true;
            return found;
        }
};

int main(){
    Derived ob(12);
    std::cout<<ob.get()<<" "<<ob.foo();
    return 0;
}