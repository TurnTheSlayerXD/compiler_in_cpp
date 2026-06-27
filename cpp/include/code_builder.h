
#ifndef CODE_BUILDER_H
#define CODE_BUILDER_H

#include <node.h>
#include <help.h>
#include <context.h>
#include <instructions.h>

UsedType* parse_typeuse(Node *n, Context &ctx, bool *err) {
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

    return tp;
}


VarDecl parse_var_decl(Node *n, Context &ctx, bool *err) {
    assert(false && "NOT IMPLEMENTED");
    assert(n->type == NodeType::VarDecl);
    UsedType* type = parse_typeuse(n->child(0), ctx, err);
    assert(n->child(1)->type == NodeType::Leaf && n->child(1)->tok().type == TokenType::WORD);
    std::string_view name = n->child(1)->text();
    return { .varName = name, .varType = type };
}

class Handler {
public:
    virtual bool can_handle(Node *n);
    virtual void interpret(Node *n, Context &ctx, bool *err);
    virtual ~Handler();
};

class FunDeclHandler: public Handler {

    std::vector<VarDecl> _paramDecls;
    std::string_view _funName;
    UsedType *_funType = nullptr;

    bool can_handle(Node *n) override {
        return n->type == NodeType::FunDecl;
    }

    bool is_fun_with_body(Node *n) {
        assert(n->type == NodeType::FunDecl);
        return n->child(3)->type != NodeType::Leaf || n->child(3)->tok().type != TokenType::SEMICOLON;
    }

    void parse_fun_type_part(Node *n, Context &ctx, bool* err) {
        
        assert(n->type == NodeType::FunDecl);

        UsedType* retType;
        std::vector<UsedType*> paramTypes;

        if (!n->child(0) || n->child(0)->type != NodeType::TypeUse) {
            *err = true;
            return;
        }

        retType = parse_typeuse(n->child(0), ctx, err); 
        if (*err) {
            return;
        }

        assert(n->child(1) && n->child(1)->type == NodeType::Leaf);

        _funName = n->child(1)->text();

        assert(n->child(2) && n->child(2)->type == NodeType::Op_Call_Brace);
        
        auto inBrace = n->child(2)->child(1);

        if (inBrace && inBrace->type == NodeType::FunDeclParams) {

            auto firstParam = inBrace->child(0);
            assert(firstParam && (firstParam->type == NodeType::VarDecl || firstParam->type == NodeType::TypeUse));

            UsedType* paramType;
            if (firstParam->type == NodeType::VarDecl) {
                auto varDecl = parse_var_decl(firstParam, ctx, err);
                paramType = varDecl.varType;
                _paramDecls.push_back(varDecl);
                if (*err) return;
            }
            else {
                paramType = parse_typeuse(firstParam, ctx, err);
                if (*err) return;
            }

            paramTypes.push_back(paramType);

            auto commaSeqParent = inBrace->child(1);
            if (commaSeqParent) {
                assert(commaSeqParent->type == NodeType::Any);
                for (auto commaJoined: commaSeqParent->children) {
                    assert(commaJoined->type == NodeType::Op_Comma);
                    UsedType *paramType;
                    if (commaJoined->child(1)->type == NodeType::VarDecl) {
                        auto varDecl = parse_var_decl(commaJoined->child(1), ctx, err);
                        if (*err) return;
                        _paramDecls.push_back(varDecl);
                        paramType = varDecl.varType;
                    }
                    else {
                        assert(commaJoined->child(1)->type == NodeType::TypeUse);
                        paramType = parse_typeuse(commaJoined->child(1), ctx, err);
                        if (*err) return;
                    }

                    paramTypes.push_back(paramType);
                }
            }
        }

        _funType = ctx.fun_type(retType, std::move(paramTypes));
    }

    void interpret(Node *n, Context &ctx, bool* err) override {
        *err = false;
        assert(n->type == NodeType::FunDecl);

        ctx.set_cursor(n->get_first_cursor());

        parse_fun_type_part(n, ctx, err);
        if (*err) {
            return;
        }

        bool hasBody = is_fun_with_body(n);
        ctx.add_fun(_funName, _funType, hasBody, err);
        if (hasBody) {
            ctx.add_mark(_funName);
        }
        if (*err) {
            *err = true;
            return;
        }        
        if (hasBody) {
            if (_funType->_f.Fun.paramCount != _paramDecls.size()) {
                *err = true;
                return;
            }
            for (size_t i = 0; i < _paramDecls.size(); ++i) {
                if (_paramDecls[i].varType != _funType->_f.Fun.paramTypes[i]) {
                    *err = true;
                    return;
                }
            }
            ctx.push_scope();
            extract_params(ctx, err);
            if (*err) {
                return;
            }
            ctx.pop_scope();
        }
    }

    void extract_params(Context &ctx, bool *err) {
        using enum Register;
        using enum InstrType;

        constexpr std::array<Register, 4> paramRegs = {
            P_REG_4,
            P_REG_3,
            P_REG_2,
            P_REG_1,
        };

        ctx.take_registers(paramRegs[0], paramRegs[1], paramRegs[2], paramRegs[3]);

        int regI = 0;

        assert(_funType->_class == UsedTypeClass::Fun);
        
        if (_funType->ret_type()->type_size() > PTR_SIZE) {
            auto retAddrP = ctx.stack_alloc(PTR_SIZE);
            ctx.add_i(MOV, paramRegs[regI], retAddrP);
            ctx.set_ret_addr_p(retAddrP);
            regI += 1;
        }   

        // First cycle should move params from registers to stack without taking into consideration sizes of types
        for (auto d: _paramDecls) {
            VarLoc p = {.decl = d, .stackP = ctx.stack_alloc(PTR_SIZE)};
            ctx.add_var(p, err);
            if (*err) {
                return;
            }
            if (regI >= paramRegs.size()) {
                auto reg = ctx.get_free_reg();
                ctx.add_i(MOV, reg_off(STACK_REG, FUN_PARAMS_STACK_OFF), reg);
                ctx.add_i(MOV, reg, p.stackP);
                ctx.free_registers(reg);
            }
            else {
                ctx.add_i(MOV, P_REG_1, p.stackP);
                regI += 1;
            }
        }

        ctx.free_registers(paramRegs[0], paramRegs[1], paramRegs[2], paramRegs[3]);
    }
};

class CurlyScopeHandler: public Handler {

};

class HandleGlobalScope: public Handler {
    bool can_handle(Node *n) {
        (void)n;
        static_assert(false && "NOT IMPLEMENTED");
    }
};


#endif