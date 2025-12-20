#include <iostream>
#include <algorithm>
#include <type_traits>

int main(){
    std::pair<std::string,int>member[]{
        {"int a", sizeof(int)},
        {"double b", sizeof(double)},
        {"bool c", sizeof(bool)},
        {"long long d", sizeof(long long)}
    };

    int n=4;
    std::cout<<"Before:";
    for (size_t i=0; i<n; ++i)
        std::cout<<member[i].first<<", ";

    std::sort(member, member+n, [](auto a, auto b){
        return a.second>b.second;
    });

    std::cout<<"\nAfter:";
    for (size_t i=0; i<n; ++i)
        std::cout<<member[i].first<<", ";
    return 0;
}

/*struct random{
    double a;
    int b;
    short c;
    bool d;
};

struct correct{
    alignas(8) double a;
    int b;
    short c;
    bool d;
};

int main(){
    std::cout<<"Random:"<<sizeof(random)<<std::endl<<alignof(random);
    std::cout<<"\nCorrect:"<<sizeof(correct)<<std::endl<<alignof(correct);
    return 0;
}*/