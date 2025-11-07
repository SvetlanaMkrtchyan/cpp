#include <iostream>

class Vector {
    private:
        int* data;  
        int size;       
        int capacity;  

        void resize(int new_cap) {
            int* temp=new int[new_cap];
            for (size_t i=0; i<size; ++i)
                temp[i]=data[i];
            
            delete[] data;
            data=temp;
            capacity=new_cap;
        }

    public:
        Vector() {
            data=new int[1];
            size=0;
            capacity=1;
        }

        ~Vector() {
            delete[] data;
            data=nullptr;
        }

        void push_back(int value) {
            if (size==capacity)
                resize(capacity*2);
            data[size++]=value;
        }

        void pop_back() {
            if (size>0)
                size--;
        }

        int get_size() {
            return size;
        }

        int get_capacity() {
            return capacity;
        }

        int at(int j) {
            return data[j]; 
        }
};

int main() {
    Vector v;
    v.push_back(10);
    v.push_back(20);
    v.push_back(30);

    std::cout<<"Size:"<<v.get_size()<<"\nCapacity:"<<v.get_capacity()<<std::endl;
    for (size_t i=0; i<v.get_size(); ++i)
        std::cout<<v.at(i)<<" ";
    std::cout << std::endl;

    v.pop_back();
    std::cout<<"After pop_back()\n"<<"Size:"<<v.get_size()<<"\nCapacity:"<<v.get_capacity()<<std::endl;
    for (size_t i=0; i<v.get_size(); ++i)
        std::cout<<v.at(i)<<" ";
    std::cout << std::endl;

    return 0;
}