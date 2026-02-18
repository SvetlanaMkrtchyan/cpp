#include <iostream>
//#include <string>
#include <map>
using namespace std;

int i = 0;
string s;
map<char, double> variables;

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

    if (i<s.length() && s[i]>='a' && s[i]<='z') {
        char var_name=s[i++];
        return variables[var_name]; 
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
    cout<<"Enter an expression: ";
    cin>>s;
    for(double v=0.0; v<=100.0; v+=0.01){
        for(char c='a'; c<='z'; c++)
            variables[c] = v;
        i=0;
        cout<<expression()<<endl;
    }
    return 0;
}