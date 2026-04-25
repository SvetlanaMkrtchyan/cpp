#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "executor.h"
#include "loader.h"
#include "memory.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <sstream>

using namespace std;

static vector<Instruction> lastProg;
static vector<double>      lastConst;
static map<string, int>    lastVarMap;

static void printHelp() {
    cout << "\nCommands:\n";
    cout << "  run     <code>   compile and run\n";
    cout << "  runmem  <code>   compile and run with Memory model\n";
    cout << "  debug   <code>   compile and debug step-by-step\n";
    cout << "  asm     <code>   compile and show disassembly\n";
    cout << "  meminfo          show Memory layout\n";
    cout << "  save    <file>   save last compiled program\n";
    cout << "  load    <file>   load and run exec file\n";
    cout << "  xscan   <code>   run with x in [1..100] step 0.01\n";
    cout << "  rvrunhex <hex...> run raw RV32I 32-bit instructions (space-separated)\n";
    cout << "  help             show this help\n";
    cout << "  quit             exit\n\n";
}

static void compileInput(const string& input) {
    tokenList.clear();
    curIdx = 0;
    funcTable.clear();
    funcIndex.clear();
    lastProg.clear();
    lastConst.clear();
    lastVarMap.clear();

    tokenize(input);

    auto topSeq = make_unique<ExprNode>(NodeKind::SEQUENCE_NODE, "top");
    while (!atEnd()) {
        auto s = parseStatement();
        if (s) topSeq->children.push_back(move(s));
    }

    int nextReg  = 1;
    int finalReg = compile(topSeq.get(), lastProg, lastConst, nextReg, lastVarMap);
    if (finalReg >= 0)
        lastProg.push_back({(uint32_t)OpCode::HALT, (uint32_t)finalReg, 0, 0});
}

int main() {
    cout << "RISC-V VM Compiler\n";
    cout << "Type 'help' for commands.\n\n";

    string line;
    while (true) {
        cout << "> ";
        if (!getline(cin, line)) break;
        if (line.empty()) continue;

        string cmd, rest;
        size_t sp = line.find(' ');
        if (sp != string::npos) { cmd = line.substr(0, sp); rest = line.substr(sp + 1); }
        else                    { cmd = line; }

        if (cmd == "quit" || cmd == "exit") break;
        if (cmd == "help") { printHelp(); continue; }

        try {
            if (cmd == "meminfo") {
                Memory m;
                m.dumpLayout();
            }
            else if (cmd == "run") {
                compileInput(rest);
                if (lastProg.empty()) { cout << "No code.\n"; continue; }
                dumpProgram(lastProg, lastConst, lastVarMap, "Assembly");
                for (auto& fe : funcTable) {
                    map<string, int> pmap;
                    for (int i = 0; i < (int)fe.params.size(); i++) pmap[fe.params[i]] = i;
                    dumpProgram(fe.code, fe.constPool, pmap, "Function: " + fe.name);
                }
                vector<double> varValues(max((int)lastVarMap.size() + 32, 64), 0.0);
                double regs[NUM_REGS] = {};
                double result = execute(lastProg, lastConst, varValues, regs, 0);
                cout << "Result = " << result << "\n";
                for (auto& [name, id] : lastVarMap)
                    if (id < (int)varValues.size())
                        cout << "  " << name << " = " << varValues[id] << "\n";
            }
            else if (cmd == "runmem") {
                compileInput(rest);
                if (lastProg.empty()) { cout << "No code.\n"; continue; }
                Memory memory;
                memory.dumpLayout();
                double result = executeWithMemory(lastProg, lastConst, memory, 0);
                cout << "Result = " << result << "\n";
                cout << "Variables (Data region):\n";
                for (auto& [name, id] : lastVarMap) {
                    uint64_t addr = memory.dataRegion.base + id * sizeof(double);
                    cout << "  " << name << " = " << memory.readDouble(addr)
                         << "  [0x" << hex << addr << dec << "]\n";
                }
            }
            else if (cmd == "debug") {
                compileInput(rest);
                if (lastProg.empty()) { cout << "No code.\n"; continue; }
                dumpProgram(lastProg, lastConst, lastVarMap, "Program");
                vector<double> varValues(max((int)lastVarMap.size() + 32, 64), 0.0);
                runDebugger(lastProg, lastConst, varValues, lastVarMap);
            }
            else if (cmd == "asm") {
                compileInput(rest);
                dumpProgram(lastProg, lastConst, lastVarMap, "Main Program");
                for (auto& fe : funcTable) {
                    map<string, int> pmap;
                    for (int i = 0; i < (int)fe.params.size(); i++) pmap[fe.params[i]] = i;
                    dumpProgram(fe.code, fe.constPool, pmap, "Function: " + fe.name);
                }
            }
            else if (cmd == "save") {
                if (lastProg.empty()) { cout << "Nothing compiled yet.\n"; continue; }
                saveExec(rest, lastProg, lastConst, lastVarMap);
                cout << "Saved to " << rest << "\n";
            }
            else if (cmd == "load") {
                ExecFile ef = loadExec(rest);
                map<string, int> varMap;
                for (int i = 0; i < (int)ef.varNames.size(); i++) varMap[ef.varNames[i]] = i;
                Memory memory;
                memory.dumpLayout();
                double result = executeWithMemory(ef.code, ef.constPool, memory, ef.header.entryPoint);
                cout << "Result = " << result << "\n";
            }
            else if (cmd == "xscan") {
                compileInput(rest);
                if (lastProg.empty()) { cout << "No code.\n"; continue; }
                dumpProgram(lastProg, lastConst, lastVarMap, "Assembly");
                for (auto& fe : funcTable) {
                    map<string, int> pmap;
                    for (int i = 0; i < (int)fe.params.size(); i++) pmap[fe.params[i]] = i;
                    dumpProgram(fe.code, fe.constPool, pmap, "Function: " + fe.name);
                }
                auto it = lastVarMap.find("x");
                if (it == lastVarMap.end()) {
                    cout << "Variable x is not defined.\n";
                    continue;
                }
                int xId = it->second;
                for (int i = 0; i <= 9900; i++) {
                    double x = 1.0 + 0.01 * i;
                    vector<double> varValues(max((int)lastVarMap.size() + 32, 64), 0.0);
                    varValues[xId] = x;
                    double regs[NUM_REGS] = {};
                    double result = execute(lastProg, lastConst, varValues, regs, 0);
                    cout << fixed << setprecision(4) << x << " " << setprecision(6) << result << "\n";
                }
            }
            else if (cmd == "rvrunhex") {
                vector<uint32_t> rawProgram;
                stringstream ss(rest);
                string tok;
                while (ss >> tok) {
                    rawProgram.push_back((uint32_t)stoul(tok, nullptr, 0));
                }
                if (rawProgram.empty()) {
                    cout << "No RV32I instructions given.\n";
                    continue;
                }
                Memory memory;
                memory.dumpLayout();
                double result = executeRV32I(rawProgram, memory, 0);
                cout << "RV32I a0 (x10) = " << result << "\n";
            }
            else {
                cout << "Unknown command. Type 'help'.\n";
            }
        } catch (const exception& e) {
            cerr << "Error: " << e.what() << "\n";
        }
    }

    return 0;
}
