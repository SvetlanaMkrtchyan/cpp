#include "instructions.h"

string opNames[] = {
    "MOV_CONST", "LOAD_VAR", "STORE_VAR",
    "ADD", "SUB", "MUL", "DIV",
    "CMP_EQ", "CMP_NEQ", "CMP_LT", "CMP_GT", "CMP_LTE", "CMP_GTE",
    "JMP", "JMP_REL", "JMP_IF_FALSE",
    "BR_GT", "BR_LT", "BR_EQ", "BR_GTE", "BR_LTE",
    "BR_GT_U", "BR_LT_U", "BR_GTE_U", "BR_LTE_U",
    "CALL", "RET", "HALT"
};

vector<FuncEntry> funcTable;
map<string, int>  funcIndex;
