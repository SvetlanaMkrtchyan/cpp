#include <iostream>

class Pointer {
    private:
        int* data;
    public:
        explicit Pointer(int value = 0) : data(new int(value)) {}

        Pointer(const Pointer& other) : data(new int(*other.data)) {}

        Pointer& operator=(const Pointer& other) {
            if (this!=&other) {
                delete data;
                data=new int(*other.data);
            }
            return *this;
        }

        Pointer(Pointer&& other) : data(other.data) {
            other.data=nullptr;
        }

        Pointer& operator=(Pointer&& other) {
            if (this!=&other) {
                delete data;
                data=other.data;
                other.data=nullptr;
            }
            return *this;
        }

        ~Pointer() {
            delete data;
        }

        Pointer operator+(const Pointer& other) const {
            return Pointer(*data+*other.data);
        }

        Pointer operator/(const Pointer& other) const {
            if (*other.data==0) 
                return Pointer(0);
            return Pointer(*data/(*other.data));
        }

        friend std::ostream& operator<<(std::ostream& os, const Pointer& p) {
            os<<*p.data;
            return os;
        }
};

int main() {
    Pointer p1(36);
    Pointer p2(18);

    Pointer p3=p1;      
    Pointer p4;
    p4=p2;         

    Pointer sum=p1+p2;
    Pointer div=p1/p2; 

    std::cout<<"p1: "<<p1<<std::endl;           
    std::cout<<"p2: "<<p2<<std::endl;             
    std::cout<<"p3: "<<p3<<std::endl; 
    std::cout<<"p4: "<<p4<<std::endl; 
    std::cout<<"sum: "<<sum<<std::endl;    
    std::cout<<"div: "<<div<<std::endl;    

    return 0;
}