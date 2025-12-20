#include <iostream>
using namespace std;

class Base {
    public:
        enum TypeID {TYPE_BASE, TYPE_DOG, TYPE_CAT };
    protected:
        TypeID typeId;
    public:
        Base() : typeId(TYPE_BASE) {}
        
        virtual void sound() { 
            cout<<"Base sound"<<endl; 
        }
        
        TypeID getTypeID() const { 
            return typeId; 
        }

        virtual bool canCastTo(TypeID target) const {
            return typeId==target;
        }

        virtual ~Base() {}
};

class Dog : public Base {
    public:
        Dog() { 
            typeId=TYPE_DOG; 
        }
        
        void sound() override { 
            cout<<"Haf-haf"<<endl; 
        }

        void fetch() { 
            cout<<"Fetching"<<endl; 
        }

        bool canCastTo(TypeID target) const override {
            return (typeId==target || target==TYPE_BASE);
        }
};

class Cat : public Base {
    public:
        Cat() { 
            typeId=TYPE_CAT; 
        }
        
        void sound() override { 
            cout<<"Meow"<<endl; 
        }
        
        void scratch() { 
            cout<<"Scratching"<<endl; 
        }

        bool canCastTo(TypeID target) const override {
            return (typeId==target || target==TYPE_BASE);
        }
};

template<typename T>
T* my_dynamic_cast(Base* src, Base::TypeID targetType) {
    if (!src) 
        return nullptr;
    
    if (src->canCastTo(targetType)) {
        return reinterpret_cast<T*>(src);
    }
    
    return nullptr;
}

int main() {
    Base* b1=new Dog();
    Base* b2=new Cat();
    Base* b3=new Base();
    
    Dog* d1=my_dynamic_cast<Dog>(b1, Base::TYPE_DOG);
    if (d1) {
        cout<<"Success";
        d1->fetch();
    }
    
    cout<<"\nCat → Dog\n";
    Dog* d2=my_dynamic_cast<Dog>(b2, Base::TYPE_DOG);
    if (!d2) {
        cout<<"Failed"<<endl;
    }
    
    cout<<"\nCat → Cat\n";
    Cat* c2=my_dynamic_cast<Cat>(b2, Base::TYPE_CAT);
    if (c2) {
        cout<<"Success\n";
        c2->scratch();
    }
    
    cout<<"\nDog → Base\n";
    Base* base=my_dynamic_cast<Base>(b1, Base::TYPE_BASE);
    if (base) {
        cout<<"Success\n";
        base->sound();
    }
    
    cout<<"\nBase → Dog\n";
    Dog* d3=my_dynamic_cast<Dog>(b3, Base::TYPE_DOG);
    if (!d3) {
        cout<<"Failed"<<endl;
    }
    
    delete b1;
    delete b2;
    delete b3;
    return 0;
}