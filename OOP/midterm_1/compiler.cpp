#include "compiler.h"
#include <stdexcept>

int compile(
    const ExprNode*      node,
    vector<Instruction>& prog,
    vector<double>&      constPool,
    int&                 nextReg,
    map<string, int>&    varMap
) {
    if (!node) return -1;

    // Map operator string → OpCode
    auto opFor = [](const string& s) -> OpCode {
        if (s == "+")  return OpCode::ADD;
        if (s == "-")  return OpCode::SUB;
        if (s == "*")  return OpCode::MUL;
        if (s == "/")  return OpCode::DIV;
        if (s == "==") return OpCode::CMP_EQ;
        if (s == "!=") return OpCode::CMP_NEQ;
        if (s == "<")  return OpCode::CMP_LT;
        if (s == ">")  return OpCode::CMP_GT;
        if (s == "<=") return OpCode::CMP_LTE;
        if (s == ">=") return OpCode::CMP_GTE;
        throw runtime_error("Unknown operator: " + s);
    };

    auto emit = [&](OpCode op, uint32_t res, uint32_t l, uint32_t r) {
        prog.push_back({(uint32_t)op, res, l, r});
    };

    auto ensureVar = [&](const string& name) -> int {
        if (!varMap.count(name)) { int id = varMap.size(); varMap[name] = id; }
        return varMap[name];
    };

    switch (node->kind) {

    // ── Literal ──────────────────────────────
    case NodeKind::NUM_NODE: {
        int r = nextReg++;
        uint32_t ci = constPool.size();
        constPool.push_back(node->val);
        emit(OpCode::MOV_CONST, r, ci, 0);
        return r;
    }

    // ── Variable load ─────────────────────────
    case NodeKind::VAR_NODE: {
        int r = nextReg++;
        emit(OpCode::LOAD_VAR, r, ensureVar(node->text), 0);
        return r;
    }

    // ── Assignment ───────────────────────────
    case NodeKind::ASSIGN_NODE: {
        int r = compile(node->left.get(), prog, constPool, nextReg, varMap);
        emit(OpCode::STORE_VAR, r, ensureVar(node->text), 0);
        return r;
    }

    // ── Binary arithmetic / comparison ───────
    case NodeKind::OP_NODE:
    case NodeKind::CMP_NODE: {
        int l = compile(node->left.get(),  prog, constPool, nextReg, varMap);
        int r = compile(node->right.get(), prog, constPool, nextReg, varMap);
        int t = nextReg++;
        emit(opFor(node->text), t, l, r);
        return t;
    }

    // ── if / else if / else ──────────────────
    case NodeKind::IF_NODE: {
        int condReg = compile(node->cond.get(), prog, constPool, nextReg, varMap);

        // Placeholder: JMP_IF_FALSE → else/end
        int jmpFalsePos = prog.size();
        emit(OpCode::JMP_IF_FALSE, condReg, 0, 0);

        // Then-body
        int thenReg = compile(node->left.get(), prog, constPool, nextReg, varMap);

        // Placeholder: JMP → end (skip else)
        int jmpEndPos = prog.size();
        emit(OpCode::JMP, 0, 0, 0);

        // Patch JMP_IF_FALSE → start of else
        prog[jmpFalsePos].leftIdx = prog.size();

        // Else-body (optional)
        int elseReg = -1;
        if (node->right)
            elseReg = compile(node->right.get(), prog, constPool, nextReg, varMap);

        // Patch JMP → here (end of if/else)
        prog[jmpEndPos].resIdx = prog.size();

        return thenReg >= 0 ? thenReg : elseReg;
    }

    // ── Statement sequence ───────────────────
    case NodeKind::SEQUENCE_NODE: {
        int last = -1;
        for (auto& child : node->children)
            last = compile(child.get(), prog, constPool, nextReg, varMap);
        return last;
    }

    // ── return ───────────────────────────────
    case NodeKind::RETURN_NODE: {
        int r = node->left ? compile(node->left.get(), prog, constPool, nextReg, varMap) : -1;
        emit(OpCode::RET, r, 0, 0);
        return r;
    }

    // ── Function definition ──────────────────
    case NodeKind::FUNC_DEF_NODE: {
        FuncEntry fe;
        fe.name   = node->text;
        fe.params = node->params;
        int reg = 0;
        map<string, int> localVarMap;
        for (int i = 0; i < (int)node->params.size(); i++) {
            localVarMap[node->params[i]] = i;
            reg++;
        }
        compile(node->left.get(), fe.code, fe.constPool, reg, localVarMap);
        fe.code.push_back({(uint32_t)OpCode::HALT, (uint32_t)(reg - 1), 0, 0});
        int idx = funcTable.size();
        funcIndex[fe.name] = idx;
        funcTable.push_back(move(fe));
        return -1;
    }

    // ── Function call ─────────────────────────
    case NodeKind::FUNC_CALL_NODE: {
        vector<int> argRegs;
        for (auto& arg : node->children)
            argRegs.push_back(compile(arg.get(), prog, constPool, nextReg, varMap));
        int resReg = nextReg++;
        if (!funcIndex.count(node->text))
            throw runtime_error("Undefined function: " + node->text);
        emit(OpCode::CALL, resReg, funcIndex[node->text], argRegs.empty() ? 0 : argRegs[0]);
        return resReg;
    }

    default: return -1;
    }
}
