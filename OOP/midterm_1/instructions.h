#pragma once
#include <vector>
#include <string>
#include <map>
#include <cstdint>
using namespace std;

enum class OpCode : uint32_t {
    MOV_CONST,     
    LOAD_VAR,       
    STORE_VAR,     
    ADD, SUB, MUL, DIV,
    CMP_EQ, CMP_NEQ, CMP_LT, CMP_GT, CMP_LTE, CMP_GTE,
    JMP,            
    JMP_IF_FALSE, 
    CALL,          
    RET,          
    HALT
};

extern string opNames[];

struct Instruction {
    uint32_t op       : 5;
    uint32_t resIdx   : 9;
    uint32_t leftIdx  : 9;
    uint32_t rightIdx : 9;
};
static_assert(sizeof(Instruction) == 4, "Instruction must be 4 bytes");

struct FuncEntry {
    string name;
    vector<string> params;
    vector<Instruction> code;
    vector<double> constPool;
};

extern vector<FuncEntry> funcTable;
extern map<string, int>  funcIndex;
