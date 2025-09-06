#include <iostream>

int main(){
    int n=7;
    int x[7]={0, 1, 3, 7, 7, 0, 1}; 
    for (int i=0; i<n; i++){
        int count=0;
        for (int j=0; j<n; j++){
            if(x[i]==x[j]){
                count++;
            }
        }
        if (count==1) {
            std::cout<<x[i];
            break;
        }
    }
    return 0;
}