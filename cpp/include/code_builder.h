
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

    enum class TypeParseState {
        INIT,
        AFTER_CONST,
    };

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

struct VarDecl {
    std::string_view varName;
    UsedType *type;
};

VarDecl parse_var_decl(Node *n, Context &ctx, bool *err) {
    assert(false && "NOT IMPLEMENTED");
    assert(n->type == NodeType::VarDecl);
    UsedType* type = parse_typeuse(n->child(0), ctx, err);
    assert(n->child(1)->type == NodeType::Leaf && n->child(1)->tok().type == TokenType::WORD);
    std::string_view name = n->child(1)->text();
    return { .varName = name, .type = type };
}

class Handler {
public:
    virtual bool can_handle(Node *n, Context &ctx);
    virtual std::vector<Instr> interpret(Node *n, Context &ctx, bool *err);
    virtual ~Handler();
};

class FunDeclHandler: public Handler {

    std::vector<VarDecl> _paramDecls;
    std::string_view _funName;

    bool can_handle(Node *n, Context &ctx) override {
        return n->type == NodeType::FunDecl;
    }

    bool is_fun_with_body(Node *n) {
        assert(n->type == NodeType::FunDecl);
        return n->child(3)->type != NodeType::Leaf || n->child(3)->tok().type != TokenType::SEMICOLON;
    }

    UsedType* parse_fun_type_part(Node *n, Context &ctx, bool* err) {
        
        assert(n->type == NodeType::FunDecl);

        UsedType* retType;
        std::vector<UsedType*> params;

        if (!n->child(0) || n->child(0)->type != NodeType::TypeUse) {
            *err = true;
            return nullptr;
        }

        retType = parse_typeuse(n->child(0), ctx, err); 
        if (*err) {
            return nullptr;
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
                paramType = varDecl.type;
                _paramDecls.push_back(varDecl);
                if (*err) return nullptr;
            }
            else {
                paramType = parse_typeuse(firstParam, ctx, err);
                if (*err) return nullptr;
            }

            params.push_back(paramType);

            auto commaSeqParent = inBrace->child(1);
            if (commaSeqParent) {
                assert(commaSeqParent->type == NodeType::Any);
                for (auto commaJoined: commaSeqParent->children) {
                    assert(commaJoined->type == NodeType::Op_Comma);
                    UsedType *paramType;
                    if (commaJoined->child(1)->type == NodeType::VarDecl) {
                        auto varDecl = parse_var_decl(commaJoined->child(1), ctx, err);
                        if (*err) return nullptr;
                        _paramDecls.push_back(varDecl);
                        paramType = varDecl.type;
                    }
                    else {
                        assert(commaJoined->child(1)->type == NodeType::TypeUse);
                        paramType = parse_typeuse(commaJoined->child(1), ctx, err);
                        if (*err) return nullptr;
                    }

                    params.push_back(paramType);
                }
            }
        }

        return ctx.fun_type(retType, std::move(params));
    }

    std::vector<Instr> interpret(Node *n, Context &ctx, bool* err) override {
        *err = false;

        assert(n->type == NodeType::FunDecl);

        UsedType* funType = parse_fun_type_part(n, ctx, err);
        if (*err) {
            return {};
        }
        
        ctx.add_var(VarDecl{ .varName = _funName, .type = funType });

        if (is_fun_with_body(n)) {
            if (funType->_f.Fun.paramCount != _paramDecls.size()) {
                *err = true;
                return {};
            }
            for (size_t i = 0; i < _paramDecls.size(); ++i) {
                if (_paramDecls[i].type != funType->_f.Fun.paramTypes[i]) {
                    *err = true;
                    return {};
                }
            }

            ctx.push_scope();



            ctx.pop_scope();
        }
    
        return {};
    }
};

class CurlyScopeHandler: public Handler {



};



class HandleGlobalScope: public Handler {

    bool can_handle(Node *n, Context &ctx) {
        assert(false && "NOT IMPLEMENTED");
    }

};


namespace GASTarget {
    std::vector<Instr> interpret(Node *root) {
        assert(false && "NOT IMPLEMENTED");
    }
}

#endif