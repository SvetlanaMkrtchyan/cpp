#include <iostream>
#include <memory>

class Student {
    private:
        std::string name;
    public:
        Student(std::string n): name(n) {}
        ~Student() {
            std::cout<<name<<" destroyed\n"; 
        }
        void print() { 
            std::cout<<name<<std::endl; 
        }
};

int main() {
    std::unique_ptr<Student> s1=std::make_unique<Student> ("Svetlana");
    s1->print(); 
} 