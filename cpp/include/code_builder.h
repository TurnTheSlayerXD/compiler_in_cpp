
#ifndef CODE_BUILDER_H
#define CODE_BUILDER_H

#include <node.h>
#include <help.h>
#include <context.h>
#include <instructions.h>
#include <charconv>

class Interpreter {
public:
    virtual bool can_handle(Node *n) = 0;
    virtual void interpret(Node *n, bool *err) = 0;
    virtual ~Interpreter() = default;
};

class TypeuseInterpreter: public Interpreter {
public:
    Context &ctx;
    UsedType *usedType;

    TypeuseInterpreter(Context &ctx): ctx{ctx} {}

    bool can_handle(Node *n) override {
        return n->type == NodeType::TypeUse;
    }

    void interpret(Node *n, bool *err) override {
        *err = false;

        ex_assert(n && "Unexpected nullptr");
        ex_assert(n->type == NodeType::TypeUse);

        bool isConst = false;
        std::string_view name;

        size_t index = 0;
        if (n->child(index) && 
            n->child(index)->type == NodeType::Leaf && 
            n->child(index)->tok().type == TokenType::KWD_CONST
        ) {
            isConst = true;
            ++index;
        }

        if (n->child(index) && 
            n->child(index)->type == NodeType::Plug) {
                ++index;
            }

        if (n->child(index) && 
            n->child(index)->type == NodeType::Leaf && 
            n->child(index)->tok().type == TokenType::WORD
        ) {
            name = n->child(index)->text();
            ++index;
        }
        
        ex_assert(index > 0 && "Failed traversing through NodeType::TypeUse");
        // if (index == 0) {
        //     *err = true;
        //     return nullptr;
        // } 
        UsedType *tp = ctx.type(name, isConst);
        
        if (n->child(index)) {
            ex_assert(n->child(index)->type == NodeType::Any);
            n = n->child(index);

            size_t index = 0;
            while (n->child(index)) {
                if (n->child(index)->type == NodeType::TypespecStar) {
                    bool isConstPtr = false;
                    if (n->child(index)->child(1)) {
                        ex_assert(n->child(index)->child(1)->type == NodeType::Leaf && n->child(index)->child(1)->tok().type == TokenType::KWD_CONST);
                        isConstPtr = true;
                    }
                    tp = ctx.ptr_type(tp, isConstPtr);
                }
                else {
                    ex_assert(false && "TODO: Other Typespecs");
                }

                ++index;
            }
        }

        usedType = tp;
    }
};

class VarDeclInterpreter: public Interpreter {
public:
    Context &ctx;
    VarDecl varDecl;

    VarDeclInterpreter(Context &ctx): ctx{ctx} {}

    bool can_handle(Node *n) override {
        return n->type == NodeType::VarDecl;
    }

    void interpret(Node *n, bool *err) override {
        ex_assert(n->type == NodeType::VarDecl);

        TypeuseInterpreter t(ctx);
        t.interpret(n->child(0), err);
        if (*err) {
            return;
        }
        UsedType* type = t.usedType;
        ex_assert(n->child(1)->type == NodeType::Leaf && n->child(1)->tok().type == TokenType::WORD);
        std::string_view name = n->child(1)->text();
        varDecl = { .varName = name, .varType = type };
    }
};

class FunDeclInterpreter: public Interpreter {
public:
    Context &ctx;
    std::vector<VarDecl> paramDecls;
    std::string_view funName;
    UsedType *funType;

    FunDeclInterpreter(Context &ctx): ctx{ctx}, funType{nullptr} {}
    
    bool can_handle(Node *n) override {
        return n->type == NodeType::FunDecl;
    }

    void interpret(Node *n, bool *err) override {
        *err = false;
        
        ex_assert(n->type == NodeType::FunDecl);

        UsedType* retType;
        std::vector<UsedType*> paramTypes;

        if (!n->child(0) || n->child(0)->type != NodeType::TypeUse) {
            *err = true;
            return;
        }

        TypeuseInterpreter t(ctx);
        t.interpret(n->child(0), err);
        if (*err) {
            return;
        }
        retType = t.usedType; 

        ex_assert(n->child(1) && n->child(1)->type == NodeType::Leaf);

        funName = n->child(1)->text();

        ex_assert(n->child(2) && n->child(2)->type == NodeType::Op_Call_Brace);
        
        auto inBrace = n->child(2)->child(1);

        if (inBrace && inBrace->type == NodeType::FunDeclParams) {

            auto firstParam = inBrace->child(0);
            ex_assert(firstParam && (firstParam->type == NodeType::VarDecl || firstParam->type == NodeType::TypeUse));

            UsedType* paramType;
            if (firstParam->type == NodeType::VarDecl) {
                VarDeclInterpreter t(ctx);
                t.interpret(firstParam, err);
                if (*err) 
                    return;
                paramType = t.varDecl.varType;
                paramDecls.push_back(t.varDecl);
            }
            else {
                TypeuseInterpreter t(ctx);
                t.interpret(firstParam, err);
                if (*err) 
                    return;
                paramType = t.usedType; 
            }

            paramTypes.push_back(paramType);

            auto commaSeqParent = inBrace->child(1);
            if (commaSeqParent) {
                ex_assert(commaSeqParent->type == NodeType::Any);
                for (auto commaJoined: commaSeqParent->children) {
                    ex_assert(commaJoined->type == NodeType::Op_Comma);
                    UsedType *paramType;
                    if (commaJoined->child(1)->type == NodeType::VarDecl) {
                        VarDeclInterpreter t(ctx);
                        t.interpret(commaJoined->child(1), err);
                        if (*err)
                            return;
                        auto varDecl = t.varDecl;
                        paramDecls.push_back(varDecl);
                        paramType = varDecl.varType;
                    }
                    else {
                        ex_assert(commaJoined->child(1)->type == NodeType::TypeUse);
                        TypeuseInterpreter t(ctx);
                        t.interpret(commaJoined->child(1), err);
                        if (*err) {
                            return;
                        }
                        paramType = t.usedType;
                    }

                    paramTypes.push_back(paramType);
                }
            }
        }

        funType = ctx.fun_type(retType, std::move(paramTypes));
    }
};

class BodyInterpreter: public Interpreter {
public:
    Context &ctx;
    BodyInterpreter(Context &ctx): ctx{ctx}{}

    bool can_handle(Node *n) override {
        (void) n;
        ex_assert(false && "SHOULD NOT BE CALLED");
        return false;
    }

    void interpret(Node *bodyNode, bool *err) override;

};

class RvalueInterpreter : public Interpreter {
public:
    Context &ctx;
    const Reg outReg;

    UsedType* outType;

    RvalueInterpreter(Context& ctx, Reg dstReg): ctx{ctx}, outReg{dstReg} {}
    bool can_handle(Node *n) override {
        return 
            n->type == NodeType::Op_Bin || 
            n->type == NodeType::Op_Un ||
            (n->type == NodeType::Leaf && (n->tok().type == TokenType::NUM_INT || n->tok().type == TokenType::NUM_FLOAT)) || 
            (n->type == NodeType::Leaf && n->tok().type == TokenType::WORD);
    }

    int parse_int(std::string_view str) {
        int result;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), result);
        ex_assert(ptr == str.data() + str.size());
        ex_assert(ec != std::errc::invalid_argument && ec != std::errc::result_out_of_range);
        return result;
    }

    void interpret(Node *n, bool *err) override {
        if (n->type == NodeType::Leaf && n->tok().type == TokenType::NUM_INT) {
            const auto &parsedInt = parse_int(n->tok().text);
            if (*err) return;
            ctx.add_i(InstrType::MOV, ctx.type("int", false)->type_size(), Lit{ .val = parsedInt }, outReg);
            outType = ctx.type("int", false);
            return;
        }
        if (n->type == NodeType::Op_Bin && n->child(1)->tok().type == TokenType::PLUS) {
            RvalueInterpreter lhs(ctx, Reg::REG_0);
            lhs.interpret(n->child(0), err);
            if (*err) return;
            StackLoc lhsLoc = ctx.stack_alloc(lhs.outType->type_size());
            ctx.add_i(InstrType::MOV, lhs.outType->type_size(), Reg::REG_0, lhsLoc);

            RvalueInterpreter rhs(ctx, Reg::REG_0);
            rhs.interpret(n->child(2), err);
            if (*err) return;

            ex_assert(lhs.outType == rhs.outType);

            ctx.add_i(InstrType::ADD, lhs.outType->type_size(), lhsLoc, Reg::REG_0);
            outType = lhs.outType;
            return;
        }
        if (n->type == NodeType::Leaf && n->tok().type == TokenType::WORD) {
            VarLoc l = ctx_get_var(ctx, n->tok().text, err);
            if (*err) {
                return;
            }
            ctx.add_i(InstrType::MOV, l.decl.varType->type_size(), l.stackLoc, outReg);
            outType = l.decl.varType;
        }

    }
};

class StatementExprInterpreter: public Interpreter {
public:
    Context &ctx;
    StatementExprInterpreter(Context &ctx): ctx{ctx}{}

    bool can_handle(Node *n) override {
        return n->type == NodeType::Statement;
    }

    void interpret(Node *n, bool *err) override {
        if (n->child(0)->type == NodeType::VarDeclWithAssign) {

            Node* expr = n->child(0); 
            Node* nodeDecl = expr->child(0);
            Node* nodeRv = expr->child(2);

            ex_assert(nodeDecl->type == NodeType::VarDecl);

            VarDeclInterpreter declInterp(ctx);
            declInterp.interpret(nodeDecl, err);
            if (*err) {
                return;
            }
            RvalueInterpreter rvInterp(ctx, Reg::REG_0);
            rvInterp.interpret(nodeRv, err);
            if (*err) {
                return;
            }
            ex_assert(rvInterp.outType == declInterp.varDecl.varType);

            VarLoc varLoc = {
                .decl = declInterp.varDecl, 
                .stackLoc = ctx.stack_alloc(declInterp.varDecl.varType->type_size()), 
                .indirect = false,
            };
            ctx.add_i(InstrType::MOV, varLoc.decl.varType->type_size(), Reg::REG_0, varLoc.stackLoc);
            ctx_set_var(ctx, varLoc, err);

            return;
        }

        ex_assert(false && "TODO");
    }
};


class ForExprInterpreter: public Interpreter {
public:
    Context &ctx;
    ForExprInterpreter(Context &ctx): ctx{ctx}{}

    bool can_handle(Node *n) override {
        return n->type == NodeType::ForStatement;
    }

    void interpret(Node *n, bool *err) override {
        ex_assert(false && "NOT IMPLEMENTED");
    }
};


class WhileExprInterpreter: public Interpreter {
public:
    Context &ctx;
    WhileExprInterpreter(Context &ctx): ctx{ctx}{}

    bool can_handle(Node *n) override {
        return n->type == NodeType::WhileStatement;
    }

    void interpret(Node *n, bool *err) override {
        ex_assert(false && "NOT IMPLEMENTED");
    }
};

class IfExprInterpreter: public Interpreter {
public:
    Context &ctx;
    IfExprInterpreter(Context &ctx): ctx{ctx}{}

    bool can_handle(Node *n) override {
        return n->type == NodeType::IfStatement;
    }

    void interpret(Node *n, bool *err) override {
        ex_assert(false && "NOT IMPLEMENTED");
    }
};

void BodyInterpreter::interpret(Node *bodyNode, bool *err) {
    IfExprInterpreter ifE(ctx);
    WhileExprInterpreter whileE(ctx);
    ForExprInterpreter forE(ctx);
    StatementExprInterpreter statementE(ctx);

    std::initializer_list<Interpreter*> interpreters = { &ifE, &forE, &statementE};

    for (auto &child: bodyNode->children) {
        bool didHandle = false;
        for (auto &interp: interpreters) {
            if (interp->can_handle(child)) {
                didHandle = true;
                interp->interpret(child, err);
                if (*err)
                    return;
                break;
            }
        }

        if (!didHandle) {
            std::fprintf(stderr, "Couldn't handle node of type %.*s", static_cast<int>(to_string(child->type).size()), to_string(child->type).data());
            ex_assert(false);
        }
    }
}


class FunInterpreter: public Interpreter {
public:
    Context &ctx;

    FunInterpreter(Context &ctx): ctx{ctx} {}

    bool can_handle(Node *n) override {
        return n->type == NodeType::FunDecl;
    }

    bool is_fun_with_body(Node *n) {
        ex_assert(n->type == NodeType::FunDecl);
        return n->child(3)->type != NodeType::Leaf || n->child(3)->tok().type != TokenType::SEMICOLON;
    }

    void interpret(Node *n, bool* err) override {
        ex_assert(n->type == NodeType::FunDecl);

        ctx.set_cursor(n->get_first_cursor());

        FunDeclInterpreter t(ctx);

        t.interpret(n, err);
        if (*err) {
            return;
        }

        bool hasBody = is_fun_with_body(n);

        auto it = std::find_if(ctx.curScope->funs.begin(), ctx.curScope->funs.end(), [&t] (const auto &v) {
            return v.decl.varName == t.funName;
        });
        if (it == ctx.curScope->funs.end()) {
            ctx.curScope->funs.push_back(FunLoc{
                .decl = {.varName = t.funName, .varType = t.funType}, 
                .wasDefined = hasBody
            });
        }
        else if ((*it).wasDefined) {
            *err = true;
            ctx.set_err(std::string("Redefinition of function with name [") + std::string((*it).decl.varName) + "]", ctx._cur);
            return;
        }
        else {
            (*it).wasDefined = hasBody;
        }

        if (hasBody) {
            ctx.add_i(Instr{
                .tp = InstrType::FUN,
                .arg1 = {.tp = InstrArgType::MARK, .data = { .mark = { .m = t.funName } }},
                .arg2 = NanInstrarg,
                .arg3 = NanInstrarg,
                .size = -1,
            });

            ctx.push_scope();
            
            int paramIndex = 0;
            if (t.funType->ret_type()->type_size() > PTR_SIZE) {
                auto retAddrP = ctx.stack_alloc(PTR_SIZE);
                ctx.add_param_i(ParamIndex{.p = paramIndex++}, retAddrP);
            }

            for (const auto &d: t.paramDecls) {
                bool indirect = d.varType->type_size() > PTR_SIZE;
                VarLoc varLoc = {.decl = d, .stackLoc = ctx.stack_alloc(PTR_SIZE), .indirect = indirect};
                ctx_set_var(ctx, varLoc, err);
                if (*err)
                    return;
                ctx.add_param_i(ParamIndex{.p = paramIndex++}, varLoc.stackLoc);
            }

            auto bodyNode = n->child(4);
            ex_assert(bodyNode->type == NodeType::Any);
            BodyInterpreter t(ctx);
            t.interpret(bodyNode, err);

            if (*err) {
                return;
            }

            ctx.pop_scope();
        }
    }

};


class BaseInterpreter: public Interpreter {
public:
    Context &ctx;

    BaseInterpreter(Context &ctx) : ctx(ctx){}

    bool can_handle(Node *n) override { return true; } 

    void interpret(Node *n, bool *err) override {
        for (const auto &c: n->children) {
            FunInterpreter t(ctx);
            t.interpret(c, err);
            if (*err) {
                break;
            }
        }
    }
};


#endif