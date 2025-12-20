#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int main() {
    //1
    auto simple=[]() {
        cout<<"Simple example of lambda!"<<endl;
    };
    simple();

    //2
    auto add=[](int a, int b) {
        return a+b;
    };
    cout<<add(3, 4)<<endl;

    //3
    int x=10;
    auto print_x=[x]() {
        cout<<x<<endl;
    };
    print_x();

    //4
    int y=20;
    auto modify_y=[&y]() {
        y+=5;
        cout<<y<<endl;
    };
    modify_y();

    //5
    vector<int> v={2, 0, 0 , 5, 4, 1, 7};
    cout<<"Original vector:";
    for (int n : v) 
        cout<<n<<" ";
    cout<<endl;
    
    sort(v.begin(), v.end(), [](int a, int b) {
        return a<b;
    });

    cout<<"Asc:";
    for (int n : v) 
        cout<<n<<" ";
    cout<<endl;

    sort(v.begin(), v.end(), [](int a, int b) {
        return a>b;
    });

    cout<<"Desc:";
    for (int n : v) 
        cout<<n<<" ";
    cout<<endl;

    //6
    auto it=find_if(v.begin(), v.end(), [](int n) {
        return (n%2==0 && n>0);
    });
    if (it!=v.end()) 
        cout<<"Even:"<<*it<<endl;
    
    return 0;
}