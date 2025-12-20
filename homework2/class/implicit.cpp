#include <iostream>

class String{
    private:
        std::string name;
    public:
        String(std::string name1): name(name1){}
        ~String(){}
        void print() const{
            std::cout<<name<<std::endl;
        }
};

int main(){
    String s("Svetlana");
    s.print();
    return 0;
}