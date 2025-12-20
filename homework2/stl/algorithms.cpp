#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <numeric>
using namespace std;

int main() {
    vector<int> v={52, 45, 23, -8, 0, 3, 12};

    cout<<"Սկզբնական vector:";
    for (int x : v) 
        cout<<x<<" ";
    cout<<"\n";

    sort(v.begin(), v.end());
    cout<<"Դասավորված vector:";
    for (int x : v) 
        cout<<x<<" ";
    cout<<"\n";

    reverse(v.begin(), v.end());
    cout<<"Շրջված vector:";
    for (int x : v) 
        cout<<x<<" ";
    cout<<"\n";

    list<int> lst(v.size());
    copy(v.begin(), v.end(), lst.begin());
    cout<<"Պատճենված list:";
    for (int x : lst) 
        cout<<x<<" ";
    cout<<"\n";

    int sum=accumulate(v.begin(), v.end(), 0);
    cout<<"Գումարը:"<<sum<<"\n";

    auto it=find(v.begin(), v.end(), 3);
    if (it!=v.end())
        cout<<"Գտնվեց արժեքը:"<<*it<<"\n";
    else
        cout<<"Արժեքը չգտնվեց\n";

    v.push_back(3);
    v.push_back(3);
    auto new_end=unique(v.begin(), v.end());
    v.erase(new_end, v.end());
    cout<<"Unique հետո vector:";
    for (int x : v) 
        cout<<x<<" ";
    cout<<"\n";

    return 0;
}