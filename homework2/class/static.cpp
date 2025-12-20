#include <iostream>
using namespace std;

class Worker {
    private:
        string name;
        double salary;
        static int count;        
        static double total; 

    public:
        Worker(string n, double s) {
            name=n;
            salary=s;
            count++;              
            total+=s;     
        }

        static int get() {  
            return count;
        }

        static double average() { 
            if (count==0) 
                return 0;
            return total/count;
        }
};

int Worker::count=0;
double Worker::total=0;

int main() {
    Worker w1("Alen", 1500);
    Worker w2("Lana", 1200);
    Worker w3("Milena", 1000);

    cout<<"Total workers:"<<Worker::get()<<endl;
    cout<<"Average salary:"<<Worker::average()<<endl;

    return 0;
}