#include <iostream>

int main(){
    double N=1000000;
    while (true){
        double next=N+1;
        if (N==next){
            std::cout<<"Ամենամեծ N-ը, որտեղ N!=(N+1)։ "<<N-1<<std::endl;
            std::cout<<"Առաջին N-ը, որտեղ N==(N+1): "<<N<<std::endl;
            break;
        }
        N++;
    }
    
    double n=1.0;
    while (n>0){
        double y=n/2;
        if (n==n+y){
            std::cout<<"Ամենափոքր n-ը, որտեղ n!=n+epsilon: "<<n*2<<std::endl;
            std::cout<<"Առաջին n-ը, որտեղ n==n+epsilon: "<<n<<std::endl;
            break;
        }
        n=y;
    }
    return 0;
}