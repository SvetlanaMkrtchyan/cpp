#include <iostream>

class Student {
    private:
        std::string name;
        int grade;
    public:
        explicit Student(std::string name1, int grade1): name(name1), grade(grade1){}
        ~Student(){}
        void print(const Student& other) {
            std::cout<<other.name<<" "<<other.grade;
        }
};
int main(){
    std::string name;
    int grade;
    Student ob(name, grade);
    ob.print(Student("Svetlana", 100));
    return 0;
}