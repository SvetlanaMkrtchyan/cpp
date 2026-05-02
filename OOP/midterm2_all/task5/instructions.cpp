#include "instructions.h"

string opNames[] = {
    "MOV_CONST", "LOAD_VAR", "STORE_VAR",
    "ADD", "SUB", "MUL", "DIV", "MOD",
    "BIT_AND", "BIT_OR", "BIT_XOR", "SHL", "SHR",
    "CMP_EQ", "CMP_NEQ", "CMP_LT", "CMP_GT", "CMP_LTE", "CMP_GTE",
    "JMP", "JMP_REL", "JMP_IF_FALSE", "JMP_IF_TRUE",
    "BR_GT", "BR_LT", "BR_EQ", "BR_GTE", "BR_LTE",
    "BR_GT_U", "BR_LT_U", "BR_GTE_U", "BR_LTE_U",
    "CALL", "RET", "HALT",
    "READ_INT", "READ_DOUBLE",
    "CMP_FLAGS", "JA", "JAE", "JB", "JBE", "JE", "JNE",
    "PUSH_BP", "MOV_BP_SP", "POP_BP", "SUB_SP",
    "LOAD_BP_OFF", "STORE_BP_OFF",
    "PUSH_ARG", "CALL_STK", "RET_STK",
};

vector<FuncEntry> funcTable;
map<string, int>  funcIndex;
