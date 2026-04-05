#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "executor.h"
#include <iostream>
#include <iomanip>
#include <stdexcept>
using namespace std;

int main() {
    cout << "Expression / Program Compiler\n";
    cout << "Supports: arithmetic, comparisons (< > == != <= >=),\n";
    cout << "          if/else if/else, function definitions and calls,\n";
    cout << "          assignments (x = ...), return, sequences.\n";
    cout << "\nEnter: ";

    string input;
    getline(cin, input);

    try {
        tokenize(input);

        auto topSeq = make_unique<ExprNode>(NodeKind::SEQUENCE_NODE, "top");
        while (!atEnd()) {
            auto s = parseStatement();
            if (s) topSeq->children.push_back(move(s));
        }

        vector<Instruction> mainProg;
        vector<double>      mainConst;
        map<string, int>    mainVarMap;
        int nextReg = 0;

        int finalReg = compile(topSeq.get(), mainProg, mainConst, nextReg, mainVarMap);
        if (finalReg >= 0)
            mainProg.push_back({(uint32_t)OpCode::HALT, (uint32_t)finalReg, 0, 0});

        dumpProgram(mainProg, mainConst, mainVarMap, "Main Program");

        for (auto& fe : funcTable) {
            map<string, int> pmap;
            for (int i = 0; i < (int)fe.params.size(); i++) pmap[fe.params[i]] = i;
            dumpProgram(fe.code, fe.constPool, pmap, "Function: " + fe.name);
        }

        if (mainProg.empty()) { 
            cout << "\nNo executable code.\n"; 
            return 0;
        }

        cout << "\nPress Enter to run for x = 1.00..100.00...";
        cin.ignore();

        vector<double> varValues(max((int)mainVarMap.size() + 10, 64), 0.0);
        double regs[1024] = {};

        cout << fixed << setprecision(4);
        for (int i = 100; i <= 10000; i++) {
            double x = i / 100.0;
            if (mainVarMap.count("x")) varValues[mainVarMap["x"]] = x;
            double result = execute(mainProg, mainConst, varValues, regs, 0);
            cout << "x = " << setw(7) << x << " | Result = " << setw(12) << result << "\n";
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}