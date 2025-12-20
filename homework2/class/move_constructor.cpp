#include <iostream>
#include <vector>
using namespace std;

class Array {
    private:   
        int* data;
        size_t size;
    public:
        Array(size_t n):size(n) {
            data=new int[n];
            cout<<"Constructor\n";
        }

        ~Array() {
            delete[] data;
            cout<<"Destructor\n";
        }

        Array(const Array& other):size(other.size) {
            data=new int[size];
            for (size_t i=0; i<size; ++i) 
                data[i]=other.data[i];
            cout<<"Copy Constructor\n";
        }

        Array(Array&& other) noexcept:data(nullptr), size(0) {
            data=other.data;  
            size=other.size;
            other.data=nullptr; 
            other.size=0;
            cout<<"Move Constructor\n";
        }
};

int main() {
    Array a(5);         
    Array b=a;  
    Array c=Array(10); 
    return 0;
}