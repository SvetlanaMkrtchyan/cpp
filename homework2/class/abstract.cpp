#include <iostream>
#include <cmath>

class Base {
    protected:
        double x;
    public:
        Base(double x1=0.0) : x(x1) {}
        virtual ~Base() {};
        virtual int calculate()=0;
    };

class Derived : public Base {
    public:
        Derived(double x2) : Base(x2) {}
        ~Derived(){}
        int calculate() override {
            return (x<0) ? static_cast<int>(x*x) : 0;
        }
};

int main() {
    double data=-3.5;
    Base* ptr=new Derived(data);

    std::cout<<ptr->calculate();  

    delete ptr;
    return 0;
}