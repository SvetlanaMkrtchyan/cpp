#include <iostream>
using namespace std;

class Base {
    public:
        virtual void foo1() {
            cout<<"Base::foo1()"<<endl;
        }
        
        virtual void foo2() {
            cout<<"Base::foo2()"<<endl;
        }
        
        void nonVirtualFoo() {
            cout<<"Base::nonVirtualFoo()"<<endl;
        }
        
        virtual ~Base() {
            cout<<"Base destructor"<<endl;
        }
};

class Derived : public Base {
    public:
        void foo1() override {
            cout<<"Derived::foo1()"<<endl;
        }
        
        virtual void foo3() {
            cout<<"Derived::foo3()"<<endl;
        }
        
        ~Derived() {
            cout<<"Derived destructor"<<endl;
        }
};

int main() {
    Base* ptr=new Derived();

    ptr->foo1(); 
    ptr->foo2(); 
    
    Derived* derivedPtr=dynamic_cast<Derived*>(ptr);
    if (derivedPtr) {
        derivedPtr->foo3();  
    }
    
    delete ptr;
    return 0;
}