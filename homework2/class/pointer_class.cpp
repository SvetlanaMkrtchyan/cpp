#include <iostream>

class Pointer {
    private:
        int* data;
    public:
        Pointer(int data1) {
            data=new int(data1);
        }

        Pointer(const Pointer& other) {
            data=new int(*other.data);
        }

        ~Pointer() {
            delete data; 
        }

        void print() const {
            std::cout<<*data<<std::endl;
        }

        Pointer& operator=(const Pointer& other) {
            if (this!=&other) {
                delete data; 
                data=new int(*other.data); 
            }
            return *this;
        }
};
int main() {
    Pointer p1(20);      
    Pointer p2=p1;     
    Pointer p3(100);
    p3=p1;           

    p1.print();
    p2.print();
    p3.print();

    return 0;
}

/*  <<Նույնը ավելի ժամանակակից ձևով>>

#include <iostream>
#include <memory>

class Pointer {
    private:
        std::unique_ptr<int> data;    
    public:
        Pointer(int data1) : data(std::make_unique<int>(data1)) {}

        void print() const {
            std::cout<<*data<<std::endl;
        }
};

int main() {
    Pointer p1(20);
    Pointer p2 = p1;   
    Pointer p3(100);
    p3 = p1;    

    p1.print(); 
    p2.print();  
    p3.print();  

    return 0;
}
 */