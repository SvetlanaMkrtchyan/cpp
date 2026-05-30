#include "instructions.h"
#include <stdexcept>

OpCode opFor(const string& s) {
    if (s == "+")   return OpCode::ADD;
    if (s == "-")   return OpCode::SUB;
    if (s == "*")   return OpCode::MUL;
    if (s == "/")   return OpCode::DIV;
    if (s == "%")   return OpCode::MOD;
    if (s == "&")   return OpCode::BIT_AND;
    if (s == "|")   return OpCode::BIT_OR;
    if (s == "^")   return OpCode::BIT_XOR;
    if (s == "<<")  return OpCode::SHL;
    if (s == ">>")  return OpCode::SHR;
    if (s == "==")  return OpCode::CMP_EQ;
    if (s == "!=")  return OpCode::CMP_NEQ;
    if (s == "<")   return OpCode::CMP_LT;
    if (s == ">")   return OpCode::CMP_GT;
    if (s == "<=")  return OpCode::CMP_LTE;
    if (s == ">=")  return OpCode::CMP_GTE;
    throw runtime_error("Unknown operator: " + s);
}

OpCode brFor(const string& s) {
    if (s == "GT")    return OpCode::BR_GT;
    if (s == "LT")    return OpCode::BR_LT;
    if (s == "EQ")    return OpCode::BR_EQ;
    if (s == "GTE")   return OpCode::BR_GTE;
    if (s == "LTE")   return OpCode::BR_LTE;
    if (s == "GT_U")  return OpCode::BR_GT_U;
    if (s == "LT_U")  return OpCode::BR_LT_U;
    if (s == "GTE_U") return OpCode::BR_GTE_U;
    if (s == "LTE_U") return OpCode::BR_LTE_U;
    throw runtime_error("Unknown branch condition: " + s);
}

int symFor(const string& name, SymKind kind, map<string, int>& varMap) {
    if (kind == SymKind::Function) {
        auto it = funcIndex.find(name);
        if (it == funcIndex.end())
            throw runtime_error("Undefined function: " + name);
        return it->second;
    }
    if (!varMap.count(name))
        varMap[name] = (int)varMap.size();
    return varMap[name];
}

string opNames[] = {
    "MOV_CONST", "LOAD_VAR", "STORE_VAR",
    "ADD", "SUB", "MUL", "DIV", "MOD",
    "BIT_AND", "BIT_OR", "BIT_XOR", "SHL", "SHR",
    "CMP_EQ", "CMP_NEQ", "CMP_LT", "CMP_GT", "CMP_LTE", "CMP_GTE",
    "JMP", "JMP_REL", "JMP_IF_FALSE", "JMP_IF_TRUE",
    "BR_GT", "BR_LT", "BR_EQ", "BR_GTE", "BR_LTE",
    "BR_GT_U", "BR_LT_U", "BR_GTE_U", "BR_LTE_U",
    "CALL", "RET", "HALT",
    "READ_INT", "READ_DOUBLE", "PRINT",
    "CMP_FLAGS", "JA", "JAE", "JB", "JBE", "JE", "JNE",
    "PUSH_BP", "MOV_BP_SP", "POP_BP", "SUB_SP",
    "LOAD_BP_OFF", "STORE_BP_OFF",
    "PUSH_ARG", "CALL_STK", "RET_STK",
};

vector<FuncEntry> funcTable;
map<string, int>  funcIndex;
