#include <iostream>
#include <map>

int main() {
    std::map<std::string, int> ages;
    ages["Svetlana"]=20;
    ages["Milena"]=21;
    ages["Venera"]=19;

    for (auto it=ages.begin(); it!=ages.end(); ++it) 
        std::cout<<it->first<<"->"<<it->second<<std::endl;
    return 0;
}