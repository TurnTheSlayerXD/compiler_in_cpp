
#ifndef CODE_BUILDER_H
#define CODE_BUILDER_H

#include <node.h>
#include <help.h>
#include <context.h>
#include <instructions.h>

class Interpreter {
public:
    virtual bool can_handle(Node *n);
    virtual void interpret(Node *n, bool *err);
    virtual ~Interpreter();
};

class TypeuseInterpreter: public Interpreter {
public:
    Context &ctx;
    UsedType *usedType;

    TypeuseInterpreter(Context &ctx): ctx{ctx} {}

    void interpret(Node *n, bool *err) {
        *err = false;

        assert(n && "Unexpected nullptr");
        assert(n->type == NodeType::TypeUse);

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
            n->child(index)->type == NodeType::Leaf && 
            n->child(index)->tok().type == TokenType::WORD
        ) {
            name = n->child(index)->text();
            ++index;
        }
        
        assert(index > 0 && "Failed traversing through NodeType::TypeUse");
        // if (index == 0) {
        //     *err = true;
        //     return nullptr;
        // } 
        UsedType *tp = ctx.type(name, isConst);
        
        if (n->child(index)) {
            assert(n->child(index)->type == NodeType::Any);
            n = n->child(index);

            size_t index = 0;
            while (n->child(index)) {
                if (n->child(index)->type == NodeType::TypespecStar) {
                    bool isConstPtr = false;
                    if (n->child(index)->child(1)) {
                        assert(n->child(index)->child(1)->type == NodeType::Leaf && n->child(index)->child(1)->tok().type == TokenType::KWD_CONST);
                        isConstPtr = true;
                    }
                    tp = ctx.ptr_type(tp, isConstPtr);
                }
                else {
                    assert(false && "TODO: Other Typespecs");
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

    void interpret(Node *n, bool *err) {
        assert(false && "NOT IMPLEMENTED");
        assert(n->type == NodeType::VarDecl);

        TypeuseInterpreter t(ctx);
        t.interpret(n->child(0), err);
        if (*err) {
            return;
        }
        UsedType* type = t.usedType;
        assert(n->child(1)->type == NodeType::Leaf && n->child(1)->tok().type == TokenType::WORD);
        std::string_view name = n->child(1)->text();
        varDecl = { .varName = name, .varType = type };
    }
};

class FunDeclInterpreter {
public:
    Context &ctx;
    std::vector<VarDecl> paramDecls;
    std::string_view funName;
    UsedType *funType;

    FunDeclInterpreter(Context &ctx): ctx{ctx}, funType{nullptr} {}

    void interpret(Node *n, bool *err) {
        *err = false;
        
        assert(n->type == NodeType::FunDecl);

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

        assert(n->child(1) && n->child(1)->type == NodeType::Leaf);

        funName = n->child(1)->text();

        assert(n->child(2) && n->child(2)->type == NodeType::Op_Call_Brace);
        
        auto inBrace = n->child(2)->child(1);

        if (inBrace && inBrace->type == NodeType::FunDeclParams) {

            auto firstParam = inBrace->child(0);
            assert(firstParam && (firstParam->type == NodeType::VarDecl || firstParam->type == NodeType::TypeUse));

            UsedType* paramType;
            if (firstParam->type == NodeType::VarDecl) {
                VarDeclInterpreter t(ctx);
                t.interpret(firstParam, err);
                if (*err) 
                    return;
                VarDecl varDecl = t.varDecl;
                paramType = varDecl.varType;
                paramDecls.push_back(varDecl);
                if (*err) 
                    return;
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
                assert(commaSeqParent->type == NodeType::Any);
                for (auto commaJoined: commaSeqParent->children) {
                    assert(commaJoined->type == NodeType::Op_Comma);
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
                        assert(commaJoined->child(1)->type == NodeType::TypeUse);
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

    void interpret(Node *bodyNode, bool *err) override;
};

class AssignExprInterpreter: public Interpreter {
public:
    Context &ctx;
    AssignExprInterpreter(Context &ctx): ctx{ctx}{}
    void interpret(Node *n, bool *err) override {
    }
};

class ForExprInterpreter: public Interpreter {
public:
    Context &ctx;
    ForExprInterpreter(Context &ctx): ctx{ctx}{}
    void interpret(Node *n, bool *err) override {
    }
};

class IfExprInterpreter: public Interpreter {
public:
    Context &ctx;
    IfExprInterpreter(Context &ctx): ctx{ctx}{}
    void interpret(Node *n, bool *err) override {
    }
};

void BodyInterpreter::interpret(Node *bodyNode, bool *err) {
    IfExprInterpreter ifE(ctx);
    ForExprInterpreter forE(ctx);
    AssignExprInterpreter assignE(ctx);

    std::initializer_list<Interpreter*> interpreters = { &ifE, &forE, &assignE};

    for (auto &child: bodyNode->children) {
        for (auto &interp: interpreters) {
            if (interp->can_handle(child)) {
                interp->interpret(child, err);
                if (*err)
                    return;

                break;
            }
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
        assert(n->type == NodeType::FunDecl);
        return n->child(3)->type != NodeType::Leaf || n->child(3)->tok().type != TokenType::SEMICOLON;
    }

    void interpret(Node *n, bool* err) override {
        assert(n->type == NodeType::FunDecl);

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
                .arg2 = NanInstrArgType,
                .arg3 = NanInstrArgType,
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
                VarLoc varLoc = {.decl = d, .stackLoc = ctx.stack_alloc(PTR_SIZE, d.varName), .indirect = indirect};
                ctx_set_var(ctx, varLoc, err);
                if (*err)
                    return;
                ctx.add_param_i(ParamIndex{.p = paramIndex++}, varLoc.stackLoc);
            }

            ctx.pop_scope();

            auto bodyNode = n->child(3);

            BodyInterpreter t(ctx);
            t.interpret(bodyNode, err);
            if (*err) {
                return;
            }
        }
    }


};



#endif