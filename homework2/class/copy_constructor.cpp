#include <iostream>

class Student{
    private:
        std::string name;
        int age;
        float grade;
    public:
        Student(std::string name1, int age1, int grade1): name(name1), age(age1), grade(grade1){}
        Student(const Student& other){
            name=other.name;
            age=other.age;
            grade=other.grade;
        }
        ~Student(){}
        float get_grade() const{
            return grade; 
        }
        void print() const{
            std::cout<<name<<" "<<age<<" "<<grade<<std::endl;
        }
};

int main(){
    Student ob1("Svetlana", 20, 100);
    Student ob2("Milena", 21, 80);
    ob1.print();
    ob2.print();
    float average=(ob1.get_grade()+ob2.get_grade())/2;
    std::cout<<"Average grade: "<<average;
    return 0;
}