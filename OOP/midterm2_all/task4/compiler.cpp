#include "compiler.h"
#include <stdexcept>

int compile(
    const ExprNode*      node,
    vector<Instruction>& prog,
    vector<double>&      constPool,
    int&                 nextReg,
    map<string, int>&    varMap,
    vector<BreakPatch>*  breakPatches
) {
    if (!node) return -1;

    auto opFor = [](const string& s) -> OpCode {
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
    };

    auto emit = [&](OpCode op, uint32_t res, uint32_t l, uint32_t r) {
        prog.push_back({(uint32_t)op, res, l, r});
    };

    auto ensureVar = [&](const string& name) -> int {
        if (!varMap.count(name)) { int id = varMap.size(); varMap[name] = id; }
        return varMap[name];
    };

    switch (node->kind) {

    case NodeKind::NUM_NODE: {
        int r = nextReg++;
        uint32_t ci = constPool.size();
        constPool.push_back(node->val);
        emit(OpCode::MOV_CONST, r, ci, 0);
        return r;
    }

    case NodeKind::VAR_NODE: {
        int r = nextReg++;
        emit(OpCode::LOAD_VAR, r, ensureVar(node->text), 0);
        return r;
    }

    case NodeKind::VAR_DECL_NODE: {
        int varId = ensureVar(node->text);
        if (node->left) {
            int r = compile(node->left.get(), prog, constPool, nextReg, varMap, breakPatches);
            emit(OpCode::STORE_VAR, r, varId, 0);
            return r;
        }
        int r = nextReg++;
        uint32_t ci = constPool.size();
        constPool.push_back(0.0);
        emit(OpCode::MOV_CONST, r, ci, 0);
        emit(OpCode::STORE_VAR, r, varId, 0);
        return r;
    }

    case NodeKind::ASSIGN_NODE: {
        int r = compile(node->left.get(), prog, constPool, nextReg, varMap, breakPatches);
        emit(OpCode::STORE_VAR, r, ensureVar(node->text), 0);
        return r;
    }

    case NodeKind::OP_NODE:
    case NodeKind::CMP_NODE: {
        int l = compile(node->left.get(),  prog, constPool, nextReg, varMap, breakPatches);
        int r = compile(node->right.get(), prog, constPool, nextReg, varMap, breakPatches);
        int t = nextReg++;
        emit(opFor(node->text), t, l, r);
        return t;
    }

    case NodeKind::INPUT_NODE: {
        int r = nextReg++;
        if (node->text == "double")
            emit(OpCode::READ_DOUBLE, r, 0, 0);
        else
            emit(OpCode::READ_INT, r, 0, 0);
        return r;
    }

    case NodeKind::IF_NODE: {
        int condReg = compile(node->cond.get(), prog, constPool, nextReg, varMap, breakPatches);
        int jmpFalsePos = prog.size();
        emit(OpCode::JMP_IF_FALSE, condReg, 0, 0);
        int thenReg = compile(node->left.get(), prog, constPool, nextReg, varMap, breakPatches);
        int jmpEndPos = prog.size();
        emit(OpCode::JMP_REL, 0, 0, 0);
        prog[jmpFalsePos].leftIdx = prog.size();
        int elseReg = -1;
        if (node->right)
            elseReg = compile(node->right.get(), prog, constPool, nextReg, varMap, breakPatches);
        prog[jmpEndPos].resIdx = (uint32_t)(int32_t)(prog.size() - (jmpEndPos + 1));
        return thenReg >= 0 ? thenReg : elseReg;
    }

    case NodeKind::WHILE_NODE: {
        vector<BreakPatch> localBreaks;
        int loopStart = prog.size();
        int condReg = compile(node->cond.get(), prog, constPool, nextReg, varMap, &localBreaks);
        int jmpExitPos = prog.size();
        emit(OpCode::JMP_IF_FALSE, condReg, 0, 0);
        compile(node->left.get(), prog, constPool, nextReg, varMap, &localBreaks);
        int backOffset = loopStart - (int)(prog.size() + 1);
        emit(OpCode::JMP_REL, (uint32_t)(int32_t)backOffset, 0, 0);
        prog[jmpExitPos].leftIdx = prog.size();
        for (auto& bp : localBreaks) prog[bp.pos].resIdx = (uint32_t)(int32_t)(prog.size() - (bp.pos + 1));
        return -1;
    }

    case NodeKind::DO_WHILE_NODE: {
        vector<BreakPatch> localBreaks;
        int loopStart = prog.size();
        compile(node->left.get(), prog, constPool, nextReg, varMap, &localBreaks);
        int condReg = compile(node->cond.get(), prog, constPool, nextReg, varMap, &localBreaks);
        int backOffset = loopStart - (int)(prog.size() + 1);
        emit(OpCode::JMP_IF_TRUE, condReg, (uint32_t)(int32_t)backOffset, 0);
        for (auto& bp : localBreaks) prog[bp.pos].resIdx = (uint32_t)(int32_t)(prog.size() - (bp.pos + 1));
        return -1;
    }

    case NodeKind::FOR_NODE: {
        vector<BreakPatch> localBreaks;
        compile(node->children[0].get(), prog, constPool, nextReg, varMap, &localBreaks);
        int loopStart = prog.size();
        int condReg = compile(node->cond.get(), prog, constPool, nextReg, varMap, &localBreaks);
        int jmpExitPos = prog.size();
        emit(OpCode::JMP_IF_FALSE, condReg, 0, 0);
        compile(node->left.get(), prog, constPool, nextReg, varMap, &localBreaks);
        compile(node->update.get(), prog, constPool, nextReg, varMap, &localBreaks);
        int backOffset = loopStart - (int)(prog.size() + 1);
        emit(OpCode::JMP_REL, (uint32_t)(int32_t)backOffset, 0, 0);
        prog[jmpExitPos].leftIdx = prog.size();
        for (auto& bp : localBreaks) prog[bp.pos].resIdx = (uint32_t)(int32_t)(prog.size() - (bp.pos + 1));
        return -1;
    }

    case NodeKind::SWITCH_NODE: {
        vector<BreakPatch> localBreaks;
        int exprReg = compile(node->left.get(), prog, constPool, nextReg, varMap, &localBreaks);
        vector<int> jmpEndPositions;
        for (auto& c : node->children) {
            if (c->kind == NodeKind::DEFAULT_NODE) {
                compile(c->left.get(), prog, constPool, nextReg, varMap, &localBreaks);
                jmpEndPositions.push_back(prog.size());
                emit(OpCode::JMP_REL, 0, 0, 0);
                continue;
            }
            int caseValReg = compile(c->cond.get(), prog, constPool, nextReg, varMap, &localBreaks);
            int cmpReg = nextReg++;
            emit(OpCode::CMP_EQ, cmpReg, exprReg, caseValReg);
            int jmpSkip = prog.size();
            emit(OpCode::JMP_IF_FALSE, cmpReg, 0, 0);
            compile(c->left.get(), prog, constPool, nextReg, varMap, &localBreaks);
            jmpEndPositions.push_back(prog.size());
            emit(OpCode::JMP_REL, 0, 0, 0);
            prog[jmpSkip].leftIdx = prog.size();
        }

        int endPos = prog.size();
        for (int p : jmpEndPositions)
            prog[p].resIdx = (uint32_t)(int32_t)(endPos - (p + 1));
        for (auto& bp : localBreaks)
            prog[bp.pos].resIdx = (uint32_t)(int32_t)(endPos - (bp.pos + 1));
        return -1;
    }

    case NodeKind::BREAK_NODE: {
        if (breakPatches) {
            breakPatches->push_back({(int)prog.size()});
            emit(OpCode::JMP_REL, 0, 0, 0);
        }
        return -1;
    }

    case NodeKind::SEQUENCE_NODE: {
        int last = -1;
        for (auto& child : node->children)
            last = compile(child.get(), prog, constPool, nextReg, varMap, breakPatches);
        return last;
    }

    case NodeKind::RETURN_NODE: {
        int r = node->left ? compile(node->left.get(), prog, constPool, nextReg, varMap, breakPatches) : -1;
        emit(OpCode::RET, r >= 0 ? r : 0, 0, 0);
        return r;
    }

    case NodeKind::FUNC_DEF_NODE: {
        FuncEntry fe;
        fe.name   = node->text;
        fe.params = node->params;
        int reg = 1;
        map<string, int> localVarMap;
        for (int i = 0; i < (int)node->params.size(); i++) {
            localVarMap[node->params[i]] = i;
            reg++;
        }
        compile(node->left.get(), fe.code, fe.constPool, reg, localVarMap, nullptr);
        fe.code.push_back({(uint32_t)OpCode::HALT, (uint32_t)(reg > 0 ? reg - 1 : 0), 0, 0});
        int idx = funcTable.size();
        funcIndex[fe.name] = idx;
        funcTable.push_back(move(fe));
        return -1;
    }

    case NodeKind::FUNC_CALL_NODE: {
        vector<int> argRegs;
        for (auto& arg : node->children)
            argRegs.push_back(compile(arg.get(), prog, constPool, nextReg, varMap, breakPatches));
        int resReg = nextReg++;
        if (!funcIndex.count(node->text))
            throw runtime_error("Undefined function: " + node->text);
        emit(OpCode::CALL, resReg, funcIndex[node->text], argRegs.empty() ? 0 : argRegs[0]);
        return resReg;
    }

    case NodeKind::BR_NODE: {
        auto brFor = [](const string& s) -> OpCode {
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
        };
        int rs1 = compile(node->left.get(),  prog, constPool, nextReg, varMap, breakPatches);
        int rs2 = compile(node->right.get(), prog, constPool, nextReg, varMap, breakPatches);
        int32_t offset = (int32_t)node->val;
        emit(brFor(node->text), rs1, rs2, (uint32_t)offset);
        return -1;
    }

    default: return -1;
    }
}
