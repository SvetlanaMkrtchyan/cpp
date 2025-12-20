/* LESS
#include <iostream>
#include <set>
using namespace std;

int main() {
    set<int, less<int>> s={2, 0, 0, 5, 4, 1};
    for (int x : s) 
        cout<<x<<" "; 
    return 0;
}*/


/* GREATER
#include <iostream>
#include <set>
using namespace std;

int main() {
    set<int, greater<int>> s={2, 0, 0, 5, 4, 1};
    for (int x : s) 
        cout<<x<<" "; 
    return 0;
}*/


/* SET (LESS/GREATER)
#include <iostream>
#include <set>
#include <functional>
using namespace std;

int main() {
    set<int, less<int>> s={2, 0, 0, 5, 4, 1};
    for (int x : s) 
        cout<<x<<" "; 
    set<int, greater<int>> l={2, 0, 0, 5, 4, 1};
    cout<<'\n';
    for (int x : l) 
        cout<<x<<" "; 
    return 0;
}*/


/* SORT (LESS/GREATER)
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int main() {
    vector<int> v={2, 0, 0, 5, 4, 1};

    sort(v.begin(), v.end(), less<int>());
    cout<<"Աճման կարգով։";
    for (int x : v) 
        cout<<x<<" ";
    cout<<endl;

    sort(v.begin(), v.end(), greater<int>());
    cout<<"Նվազման կարգով։";
    for (int x : v) 
        cout<<x<<" ";
    cout<<endl;

    return 0;
}*/