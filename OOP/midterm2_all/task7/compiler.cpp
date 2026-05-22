#include "compiler.h"
#include <cmath>
#include <stdexcept>

static int foldBinary(const string& op, double a, double b, double& out) {
    if (op == "+")  { out = a + b; return 1; }
    if (op == "-")  { out = a - b; return 1; }
    if (op == "*")  { out = a * b; return 1; }
    if (op == "/")  { out = b != 0 ? a / b : 0; return 1; }
    if (op == "%")  { out = fmod(a, b); return 1; }
    return 0;
}

int compile(
    const ExprNode*      node,
    vector<Instruction>& prog,
    vector<double>&      constPool,
    int&                 nextReg,
    map<string, int>&    varMap,
    vector<BreakPatch>*  breakPatches,
    vector<ContinuePatch>* continuePatches,
    CompileInfo*         info
) {
    if (!node) return -1;

    auto emit = [&](OpCode op, uint32_t res, uint32_t l, uint32_t r) {
        prog.push_back({(uint32_t)op, res, l, r});
        if (info) info->instrCount = (int)prog.size();
    };

    auto noteStore = [&](int reg, int varId) {
        if (!info) return;
        info->finalReg  = reg;
        info->lastVarId = varId;
    };

    switch (node->kind) {

    case NodeKind::NUM_NODE: {
        int r = nextReg++;
        uint32_t ci = constPool.size();
        constPool.push_back(node->val);
        emit(OpCode::MOV_CONST, r, ci, 0);
        if (info) info->finalReg = r;
        return r;
    }

    case NodeKind::VAR_NODE: {
        int r = nextReg++;
        emit(OpCode::LOAD_VAR, r, symFor(node->text, SymKind::Variable, varMap), 0);
        if (info) info->finalReg = r;
        return r;
    }

    case NodeKind::NOT_NODE: {
        int v = compile(node->left.get(), prog, constPool, nextReg, varMap, breakPatches, continuePatches, info);
        int r = nextReg++;
        int z = nextReg++;
        uint32_t zi = constPool.size();
        constPool.push_back(0.0);
        emit(OpCode::MOV_CONST, z, zi, 0);
        emit(OpCode::CMP_EQ, r, v, z);
        if (info) info->finalReg = r;
        return r;
    }

    case NodeKind::VAR_DECL_NODE: {
        int varId = symFor(node->text, SymKind::Variable, varMap);
        if (node->left) {
            int r = compile(node->left.get(), prog, constPool, nextReg, varMap, breakPatches, continuePatches, info);
            emit(OpCode::STORE_VAR, r, varId, 0);
            noteStore(r, varId);
            return r;
        }
        int r = nextReg++;
        uint32_t ci = constPool.size();
        constPool.push_back(0.0);
        emit(OpCode::MOV_CONST, r, ci, 0);
        emit(OpCode::STORE_VAR, r, varId, 0);
        noteStore(r, varId);
        return r;
    }

    case NodeKind::ASSIGN_NODE: {
        int r = compile(node->left.get(), prog, constPool, nextReg, varMap, breakPatches, continuePatches, info);
        int varId = symFor(node->text, SymKind::Variable, varMap);
        emit(OpCode::STORE_VAR, r, varId, 0);
        noteStore(r, varId);
        return r;
    }

    case NodeKind::OP_NODE:
    case NodeKind::CMP_NODE: {
        if (node->text == "&&" || node->text == "||") {
            int l = compile(node->left.get(), prog, constPool, nextReg, varMap, breakPatches, continuePatches, info);
            if (node->text == "&&") {
                int jmpSkip = prog.size();
                emit(OpCode::JMP_IF_FALSE, l, 0, 0);
                int r = compile(node->right.get(), prog, constPool, nextReg, varMap, breakPatches, continuePatches, info);
                int jmpEnd = prog.size();
                emit(OpCode::JMP_REL, 0, 0, 0);
                prog[jmpSkip].leftIdx = prog.size();
                int z = nextReg++;
                uint32_t zi = constPool.size();
                constPool.push_back(0.0);
                emit(OpCode::MOV_CONST, z, zi, 0);
                prog[jmpEnd].resIdx = (uint32_t)(int32_t)(prog.size() - (jmpEnd + 1));
                if (info) info->finalReg = r;
                return r;
            }
            int jmpRight = prog.size();
            emit(OpCode::JMP_IF_FALSE, l, 0, 0);
            int t = nextReg++;
            uint32_t oneIdx = constPool.size();
            constPool.push_back(1.0);
            emit(OpCode::MOV_CONST, t, oneIdx, 0);
            int jmpEnd = prog.size();
            emit(OpCode::JMP_REL, 0, 0, 0);
            prog[jmpRight].leftIdx = prog.size();
            int r = compile(node->right.get(), prog, constPool, nextReg, varMap, breakPatches, continuePatches, info);
            int z = nextReg++;
            uint32_t zIdx = constPool.size();
            constPool.push_back(0.0);
            emit(OpCode::MOV_CONST, z, zIdx, 0);
            emit(OpCode::ADD, t, r, z);
            prog[jmpEnd].resIdx = (uint32_t)(int32_t)(prog.size() - (jmpEnd + 1));
            if (info) info->finalReg = t;
            return t;
        }
        if (node->left && node->right &&
            node->left->kind == NodeKind::NUM_NODE &&
            node->right->kind == NodeKind::NUM_NODE) {
            double folded = 0;
            if (foldBinary(node->text, node->left->val, node->right->val, folded)) {
                int r = nextReg++;
                uint32_t ci = constPool.size();
                constPool.push_back(folded);
                emit(OpCode::MOV_CONST, r, ci, 0);
                if (info) info->finalReg = r;
                return r;
            }
        }
        int l = compile(node->left.get(),  prog, constPool, nextReg, varMap, breakPatches, continuePatches, info);
        int r = compile(node->right.get(), prog, constPool, nextReg, varMap, breakPatches, continuePatches, info);
        int t = nextReg++;
        emit(opFor(node->text), t, l, r);
        if (info) info->finalReg = t;
        return t;
    }

    case NodeKind::INPUT_NODE: {
        int r = nextReg++;
        if (node->text == "double")
            emit(OpCode::READ_DOUBLE, r, 0, 0);
        else
            emit(OpCode::READ_INT, r, 0, 0);
        if (info) info->finalReg = r;
        return r;
    }

    case NodeKind::PRINT_NODE: {
        int r = compile(node->left.get(), prog, constPool, nextReg, varMap, breakPatches, continuePatches, info);
        emit(OpCode::PRINT, r, 0, 0);
        return -1;
    }

    case NodeKind::IF_NODE: {
        int condReg = compile(node->cond.get(), prog, constPool, nextReg, varMap, breakPatches, continuePatches, info);
        int jmpFalsePos = prog.size();
        emit(OpCode::JMP_IF_FALSE, condReg, 0, 0);
        int thenReg = compile(node->left.get(), prog, constPool, nextReg, varMap, breakPatches, continuePatches, info);
        int jmpEndPos = prog.size();
        emit(OpCode::JMP_REL, 0, 0, 0);
        prog[jmpFalsePos].leftIdx = prog.size();
        int elseReg = -1;
        if (node->right)
            elseReg = compile(node->right.get(), prog, constPool, nextReg, varMap, breakPatches, continuePatches, info);
        prog[jmpEndPos].resIdx = (uint32_t)(int32_t)(prog.size() - (jmpEndPos + 1));
        return thenReg >= 0 ? thenReg : elseReg;
    }

    case NodeKind::WHILE_NODE: {
        vector<BreakPatch> localBreaks;
        vector<ContinuePatch> localContinues;
        int loopStart = prog.size();
        int condReg = compile(node->cond.get(), prog, constPool, nextReg, varMap, &localBreaks, &localContinues, info);
        int jmpExitPos = prog.size();
        emit(OpCode::JMP_IF_FALSE, condReg, 0, 0);
        compile(node->left.get(), prog, constPool, nextReg, varMap, &localBreaks, &localContinues, info);
        int backOffset = loopStart - (int)(prog.size() + 1);
        emit(OpCode::JMP_REL, (uint32_t)(int32_t)backOffset, 0, 0);
        prog[jmpExitPos].leftIdx = prog.size();
        for (auto& bp : localBreaks)
            prog[bp.pos].resIdx = (uint32_t)(int32_t)(prog.size() - (bp.pos + 1));
        for (auto& cp : localContinues)
            prog[cp.pos].resIdx = (uint32_t)(int32_t)(loopStart - (cp.pos + 1));
        return -1;
    }

    case NodeKind::DO_WHILE_NODE: {
        vector<BreakPatch> localBreaks;
        vector<ContinuePatch> localContinues;
        int loopStart = prog.size();
        compile(node->left.get(), prog, constPool, nextReg, varMap, &localBreaks, &localContinues, info);
        int condStart = prog.size();
        int condReg = compile(node->cond.get(), prog, constPool, nextReg, varMap, &localBreaks, &localContinues, info);
        int backOffset = loopStart - (int)(prog.size() + 1);
        emit(OpCode::JMP_IF_TRUE, condReg, (uint32_t)(int32_t)backOffset, 0);
        for (auto& bp : localBreaks)
            prog[bp.pos].resIdx = (uint32_t)(int32_t)(prog.size() - (bp.pos + 1));
        for (auto& cp : localContinues)
            prog[cp.pos].resIdx = (uint32_t)(int32_t)(condStart - (cp.pos + 1));
        return -1;
    }

    case NodeKind::FOR_NODE: {
        vector<BreakPatch> localBreaks;
        vector<ContinuePatch> localContinues;
        compile(node->children[0].get(), prog, constPool, nextReg, varMap, &localBreaks, &localContinues, info);
        int loopStart = prog.size();
        int condReg = compile(node->cond.get(), prog, constPool, nextReg, varMap, &localBreaks, &localContinues, info);
        int jmpExitPos = prog.size();
        emit(OpCode::JMP_IF_FALSE, condReg, 0, 0);
        compile(node->left.get(), prog, constPool, nextReg, varMap, &localBreaks, &localContinues, info);
        int continueTarget = prog.size();
        compile(node->update.get(), prog, constPool, nextReg, varMap, &localBreaks, &localContinues, info);
        int backOffset = loopStart - (int)(prog.size() + 1);
        emit(OpCode::JMP_REL, (uint32_t)(int32_t)backOffset, 0, 0);
        prog[jmpExitPos].leftIdx = prog.size();
        for (auto& bp : localBreaks)
            prog[bp.pos].resIdx = (uint32_t)(int32_t)(prog.size() - (bp.pos + 1));
        for (auto& cp : localContinues)
            prog[cp.pos].resIdx = (uint32_t)(int32_t)(continueTarget - (cp.pos + 1));
        return -1;
    }

    case NodeKind::SWITCH_NODE: {
        vector<BreakPatch> localBreaks;
        int exprReg = compile(node->left.get(), prog, constPool, nextReg, varMap, &localBreaks, nullptr, info);
        vector<int> jmpEndPositions;
        for (auto& c : node->children) {
            if (c->kind == NodeKind::DEFAULT_NODE) {
                compile(c->left.get(), prog, constPool, nextReg, varMap, &localBreaks, nullptr, info);
                jmpEndPositions.push_back(prog.size());
                emit(OpCode::JMP_REL, 0, 0, 0);
                continue;
            }
            int caseValReg = compile(c->cond.get(), prog, constPool, nextReg, varMap, &localBreaks, nullptr, info);
            int cmpReg = nextReg++;
            emit(OpCode::CMP_EQ, cmpReg, exprReg, caseValReg);
            int jmpSkip = prog.size();
            emit(OpCode::JMP_IF_FALSE, cmpReg, 0, 0);
            compile(c->left.get(), prog, constPool, nextReg, varMap, &localBreaks, nullptr, info);
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
        } else {
            throw runtime_error("'break' outside of loop");
        }
        return -1;
    }

    case NodeKind::CONTINUE_NODE: {
        if (continuePatches) {
            continuePatches->push_back({(int)prog.size()});
            emit(OpCode::JMP_REL, 0, 0, 0);
        } else {
            throw runtime_error("'continue' outside of loop");
        }
        return -1;
    }

    case NodeKind::SEQUENCE_NODE: {
        int last = -1;
        for (auto& child : node->children)
            last = compile(child.get(), prog, constPool, nextReg, varMap, breakPatches, continuePatches, info);
        return last;
    }

    case NodeKind::RETURN_NODE: {
        int r = node->left ? compile(node->left.get(), prog, constPool, nextReg, varMap, breakPatches, continuePatches, info) : -1;
        int retReg = (r >= 0 ? r : 0);
        emit(OpCode::POP_BP, 0, 0, 0);
        emit(OpCode::RET, retReg, 0, 0);
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

        auto emitF = [&](OpCode op, uint32_t res, uint32_t l, uint32_t r) {
            fe.code.push_back({(uint32_t)op, res, l, r});
        };

        emitF(OpCode::PUSH_BP,   0, 0, 0);
        emitF(OpCode::MOV_BP_SP, 0, 0, 0);

        int bodyStartSize = fe.code.size();
        compile(node->left.get(), fe.code, fe.constPool, reg, localVarMap, nullptr, nullptr, nullptr);

        int numLocals = (int)localVarMap.size() - (int)node->params.size();
        if (numLocals < 0) numLocals = 0;
        Instruction subSpInst = {(uint32_t)OpCode::SUB_SP, (uint32_t)numLocals, 0, 0};
        fe.code.insert(fe.code.begin() + bodyStartSize, subSpInst);

        bool hasRet = false;
        for (auto& ins : fe.code)
            if ((OpCode)ins.op == OpCode::RET) { hasRet = true; break; }
        if (!hasRet) {
            emitF(OpCode::POP_BP, 0, 0, 0);
            fe.code.push_back({(uint32_t)OpCode::HALT, (uint32_t)(reg > 0 ? reg - 1 : 0), 0, 0});
        }

        int idx = funcTable.size();
        funcIndex[fe.name] = idx;
        funcTable.push_back(move(fe));
        return -1;
    }

    case NodeKind::FUNC_CALL_NODE: {
        vector<int> argRegs;
        for (auto& arg : node->children)
            argRegs.push_back(compile(arg.get(), prog, constPool, nextReg, varMap, breakPatches, continuePatches, info));
        int resReg = nextReg++;
        int fIdx = symFor(node->text, SymKind::Function, varMap);
        int numArgs = (int)argRegs.size();
        for (int i = numArgs - 1; i >= 0; i--)
            emit(OpCode::PUSH_ARG, argRegs[i], 0, 0);
        emit(OpCode::CALL, resReg, fIdx, argRegs.empty() ? 0 : argRegs[0]);
        if (info) info->finalReg = resReg;
        return resReg;
    }

    case NodeKind::BR_NODE: {
        int rs1 = compile(node->left.get(),  prog, constPool, nextReg, varMap, breakPatches, continuePatches, info);
        int rs2 = compile(node->right.get(), prog, constPool, nextReg, varMap, breakPatches, continuePatches, info);
        int32_t offset = (int32_t)node->val;
        emit(brFor(node->text), rs1, rs2, (uint32_t)offset);
        return -1;
    }

    default: return -1;
    }
}
