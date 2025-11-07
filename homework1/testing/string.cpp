#include <iostream>
#include <cstring>

class String {
    private:
        char* data;
        int size;
        int capacity;

        void expand(int newCap) {
            if (newCap>capacity) {
                char* newData=new char[newCap+1];
                strcpy(newData, data);
                delete[] data;
                data=newData;
                capacity=newCap;
            }
        }

    public:
        String(const char* str="") {
            size=strlen(str);
            capacity=size;
            data=new char[capacity+1];
            strcpy(data, str);
        }

        String(const String& obj) {
            size=obj.size;
            capacity=obj.capacity;
            data=new char[capacity+1];
            strcpy(data, obj.data);
        }

        ~String() {
            delete[] data;
            data=nullptr;
        }

        void add(const char* str) {
            int len=strlen(str);
            expand(size+len);
            strcat(data, str);
            size+=len;
        }

        void print() const{
            std::cout<<data;
        }

        int len() const{ 
            return size; 
        }

        int cap() const{ 
            return capacity; 
        }
    };

int main() {
    String s("Svetlana");
    s.add(" Mkrtchyan");
    std::cout<<"Text:";
    s.print();
    std::cout<<std::endl;
    std::cout<<"Size:"<<s.len()<<std::endl;
    std::cout<<"Capacity:"<<s.cap()<<std::endl;

    return 0;
}