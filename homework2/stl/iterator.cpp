/* LIST

#include <iostream>
#include <list>

int main() {
    std::list<std::string> names={"Svetlana", "Milena", "Venera"};

    for (std::list<std::string>::iterator it=names.begin(); it!=names.end(); ++it) 
        std::cout<<*it<<" ";
    return 0;
}*/


/* VECTOR

#include <iostream>
#include <vector>

int main() {
    std::vector<int> v={120, 40, 76, 23};

    std::vector<int>::iterator it;

    for (it=v.begin(); it!=v.end(); ++it)
        std::cout<<*it<<" ";  
    return 0;
}*/