#include "instructions.h"

string opNames[] = {
    "MOV_CONST", "LOAD_VAR", "STORE_VAR",
    "ADD", "SUB", "MUL", "DIV",
    "CMP_EQ", "CMP_NEQ", "CMP_LT", "CMP_GT", "CMP_LTE", "CMP_GTE",
    "JMP", "JMP_IF_FALSE", "CALL", "RET", "HALT"
};

vector<FuncEntry> funcTable;
map<string, int>  funcIndex;
