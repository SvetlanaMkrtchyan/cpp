#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <iomanip>
using namespace std;

enum OpCode { ADD, SUB, MUL, DIV };

struct Instruction {
    OpCode op;
    int resIdx;   
    int leftIdx;  
    int rightIdx; 
};

struct VMState {
    double registers[1024]={0};
    int nextReg=0;
};

enum TokenType { NUMBER, VARIABLE, OPERATOR, LPAREN, RPAREN, END };
struct Token {
    TokenType type;
    string value;
};

enum NodeKind { NUM_NODE, VAR_NODE, OP_NODE };
struct ExprNode {
    NodeKind kind;
    string text;
    double val;
    ExprNode *left=nullptr, *right=nullptr;

    ExprNode(NodeKind k, string t) : kind(k), text(t), val(0) {}
    ExprNode(double v) : kind(NUM_NODE), text(""), val(v) {}
    ~ExprNode() { 
        delete left; 
        delete right; 
    }
};

vector<Token> tokenList;
int curIdx=0;

void tokenize(string input) {
    tokenList.clear(); curIdx=0;
    for (int i=0; i<input.length(); ++i) {
        if (isspace(input[i])) 
            continue;
        if (isdigit(input[i])) {
            string t; 
            while (i<input.length() && (isdigit(input[i]) || input[i] == '.')) 
                t+=input[i++];
            tokenList.push_back({NUMBER, t}); 
            i--;
        } 
        else if (isalpha(input[i])) {
            string t; 
            while (i<input.length() && isalnum(input[i])) 
                t+=input[i++];
            tokenList.push_back({VARIABLE, t}); 
            i--;
        } 
        else {
            TokenType type=(input[i]=='(') ? LPAREN : (input[i] == ')') ? RPAREN : OPERATOR;
            tokenList.push_back({type, string(1, input[i])});
        }
    }
}

ExprNode* parseExpr();
ExprNode* parsePrimary() {
    if (curIdx>=tokenList.size()) 
        return nullptr;
    Token t=tokenList[curIdx++];
    if (t.type==NUMBER) 
        return new ExprNode(stod(t.value));
    if (t.type==VARIABLE) 
        return new ExprNode(VAR_NODE, t.value);
    if (t.type==LPAREN) { 
        ExprNode* n=parseExpr(); 
        curIdx++; 
        return n; 
    }
    return nullptr;
}

ExprNode* parseTerm() {
    ExprNode* l=parsePrimary();
    while (curIdx<tokenList.size() && (tokenList[curIdx].value=="*" || tokenList[curIdx].value=="/")) {
        string op=tokenList[curIdx++].value;
        ExprNode* p=new ExprNode(OP_NODE, op); 
        p->left=l; 
        p->right=parsePrimary(); 
        l=p;
    }
    return l;
}

ExprNode* parseExpr() {
    ExprNode* l=parseTerm();
    while (curIdx<tokenList.size() && (tokenList[curIdx].value=="+" || tokenList[curIdx].value=="-")) {
        string op=tokenList[curIdx++].value;
        ExprNode* p=new ExprNode(OP_NODE, op); 
        p->left=l; 
        p->right=parseTerm(); 
        l=p;
    }
    return l;
}

int compile(ExprNode* node, vector<Instruction>& program, VMState& vm, map<string, int>& varMap) {
    if (!node) 
        return -1;
    if (node->kind==NUM_NODE) {
        int r=vm.nextReg++;
        vm.registers[r]=node->val;
        return r;
    }
    if (node->kind==VAR_NODE) {
        if (varMap.find(node->text)==varMap.end()) 
            varMap[node->text]=vm.nextReg++;
        return varMap[node->text];
    }
    int l=compile(node->left, program, vm, varMap);
    int r=compile(node->right, program, vm, varMap);
    int target=vm.nextReg++;
    OpCode op=(node->text == "+") ? ADD : (node->text=="-") ? SUB : (node->text=="*") ? MUL : DIV;
    program.push_back({op, target, l, r});
    return target;
}

double execute(const vector<Instruction>& program, VMState& vm, bool printTrace) {
    string ops[]={"ADD", "SUB", "MUL", "DIV"};
    for (const auto& inst : program) {
        double valLeft=vm.registers[inst.leftIdx];
        double valRight=vm.registers[inst.rightIdx];
        
        if (inst.op==ADD) 
            vm.registers[inst.resIdx]=valLeft+valRight;
        else if (inst.op==SUB) 
            vm.registers[inst.resIdx]=valLeft-valRight;
        else if (inst.op==MUL) 
            vm.registers[inst.resIdx]=valLeft*valRight;
        else if (inst.op==DIV) 
            vm.registers[inst.resIdx]=(valRight!=0) ? valLeft/valRight : 0;
        if (printTrace) {
            cout<<"  "<<ops[inst.op]<<" r"<<inst.resIdx<<", r"<<inst.leftIdx<<", r"<<inst.rightIdx;
            cout<<" => ("<<valLeft<<(inst.op==ADD ? "+" : inst.op==SUB ? "-" : inst.op==MUL ? "*" : "/")<<valRight<<") = "<<vm.registers[inst.resIdx]<<endl;
        }
    }
    return program.empty() ? 0 : vm.registers[program.back().resIdx];
}

int main() {
    string input;
    cout<<"Enter:";
    getline(cin, input);

    tokenize(input);
    ExprNode* root=parseExpr();

    if (root) {
        VMState vm;
        vector<Instruction> program;
        map<string, int> varMap;

        compile(root, program, vm, varMap);
        
        bool firstRun=true;
        for (double x=1.0; x<=100.05; x+=0.1) { 
            if (varMap.count("x")) 
                vm.registers[varMap["x"]]=x;
            if (firstRun) {
                cout<<"\n(x="<<x<<"):\n";
                execute(program, vm, true);
                cout<<"\nThe rest\n";
                firstRun=false;
            } 
            else {
                double result=execute(program, vm, false);
                cout<<fixed<<setprecision(1)<<"x = "<<x<<" => Result: "<<setprecision(4)<<result<<endl;
            }
        }
    }
    delete root;
    return 0;
}