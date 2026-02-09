#include <iostream>
#include <string>
using namespace std;

int i = 0;
string s;

double number();
double factor();
double expression();
double term();

double number(){
    double result=0;
    while(i<s.length() && s[i]>='0' && s[i]<='9'){
        result=result*10+(s[i]-'0');
        i++;
    }
    return result;
}

double factor(){
    if(s[i]=='('){
        i++;
        double result=expression();
        i++;
        return result;
    }
    return number();
}

double term(){
    double result=factor();
    while(i<s.length() && s[i]=='*' || s[i]=='/'){
        char op=s[i++];
        if(op=='*')
            result*=factor();
        else
            result/=factor();
    }
    return result;
}

double expression(){
    double result=term();
    while(i<s.length() && s[i]=='+' || s[i]=='-'){
        char op=s[i++];
        if(op=='+')
            result+=term();
        else
            result-=term();
    }
    return result;
}

int main (){
    cin>>s;
    i=0;
    cout<<expression()<<endl;
    return 0;
}