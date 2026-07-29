#ifndef CONTEXT_H
#define CONTEXT_H

#include <instructions.h>
#include <used_type.h>
#include <help.h>

struct VarDecl {
    std::string_view varName;
    UsedType *varType;
};

struct VarLoc {
    VarDecl decl;
    StackLoc stackLoc;
    bool indirect;
};

struct FunLoc {
    VarDecl decl;
    bool wasDefined;
};

class Scope {
    std::vector<std::array<char, 20>> _markHeap;

public:
    std::string id;
    int markCounter;
    int stackOff;

    Scope* parentScope;
    std::vector<Scope*> subscopes;

    std::vector<VarLoc> vars;
    std::vector<FunLoc> funs;

    std::vector<UsedType*> types;

    Scope(Scope *parentScope): markCounter{0}, stackOff{0}, parentScope{parentScope} {
        if (parentScope) {
            id = str_fmt("%s_%llu", parentScope->id.c_str(), parentScope->subscopes.size());
            parentScope->subscopes.push_back(this);
        }
        else {
            id = "OUTER";
        }
    }

    Mark next_mark() {
        _markHeap.push_back({});
        auto &s = _markHeap.back();
        std::snprintf(s.data(), s.size(), "sc_%s_m_%d", id.c_str(), markCounter++);
        return Mark{.m =  std::string_view(s.data(), s.size())};
    }

    Scope(const Scope& other) = delete;
    Scope(Scope&& other) = delete;

    Scope& operator=(const Scope& other) = delete;
    Scope& operator=(Scope&& other) = delete;
};

class Context {
public:

    std::vector<Instr> _instrs;

    std::vector<UsedType*> _typeSt; 
    std::vector<Scope*> _scopeSt; 

    Scope *curScope;

    std::vector<std::string> _errs;

    const Cursor* _cur;

    Context() 
    : _typeSt {
        new UsedType({ ._class = UsedTypeClass::Type, ._f = { .Type = {.name = "int",  .isConst = false, .isStructKind = false, .fields = nullptr, .fieldCount = 0 } }}),
        new UsedType({ ._class = UsedTypeClass::Type, ._f = { .Type = {.name = "void",  .isConst = false, .isStructKind = false, .fields = nullptr, .fieldCount = 0 } }}),
        new UsedType({ ._class = UsedTypeClass::Type, ._f = { .Type = {.name = "char",  .isConst = false, .isStructKind = false, .fields = nullptr, .fieldCount = 0 } }}),
    }, _cur{nullptr}

    {
        curScope = new Scope(nullptr);
        _scopeSt.push_back(curScope);
    }

    Context(const Context& rhs) = delete;
    Context(Context&& rhs) = delete;

    Context& operator=(const Context& rhs) = delete;
    Context& operator=(Context&& rhs) = delete;

    UsedType* type(std::string_view nm, bool isConst) {
        auto it = std::find_if(_typeSt.begin(), _typeSt.end(), [&nm, &isConst](const auto &arg) { 
            if (arg->_class != UsedTypeClass::Type) {
                return false;
            }
            return arg->_f.Type.name == nm && arg->_f.Type.isConst == isConst; 
        });
        if (it == _typeSt.end()) {
            UsedType* newPtr = new UsedType({ 
                ._class = UsedTypeClass::Type, 
                ._f = { 
                    .Type = { 
                        .name = nm, 
                        .isConst = isConst, 
                        .isStructKind = false, 
                        .fields = nullptr, 
                        .fieldCount = 0 
                    }
                }
            });
            _typeSt.push_back(newPtr);
            return newPtr;
        }
        return *it;
    }

    void set_err(std::string &&msg, const Cursor* cur) {
        ex_assert(cur);
        _errs.push_back(std::string("Error at") + to_string(*cur) + msg);
    }

    UsedType* ptr_type(UsedType *ptrTo, bool isConstPtr) {
        auto it = std::find_if(_typeSt.begin(), _typeSt.end(), [&ptrTo, &isConstPtr](const auto &arg) { 
            if (arg->_class != UsedTypeClass::Ptr) {
                return false;
            }
            return ptrTo == arg->_f.Ptr.ptrTo && isConstPtr == arg->_f.Ptr.isConstPtr;
        });
        if (it == _typeSt.end()) {
            UsedType* newPtr = new UsedType({ ._class=UsedTypeClass::Ptr, ._f = { .Ptr = {.ptrTo = ptrTo, .isConstPtr = isConstPtr}} });
            _typeSt.push_back(newPtr);
            return newPtr;
        }

        return *it;
    }

    UsedType* fun_type(UsedType* retType, std::vector<UsedType*> &&paramTypes) {
        auto it = std::find_if(_typeSt.begin(), _typeSt.end(), [&retType, &paramTypes](const auto &arg) { 
            if (arg->_class != UsedTypeClass::Fun) {
                return false;
            }
            if (retType != arg->_f.Fun.retType) return false;
            if (paramTypes.size() != arg->_f.Fun.paramCount) return false;
            for (size_t i = 0; i < paramTypes.size(); ++i) {
                if (paramTypes[i] != arg->_f.Fun.paramTypes[i]) 
                    return false;
            }
            return true;    
        });

        if (it == _typeSt.end()) {
            UsedType tp = { ._class=UsedTypeClass::Fun, ._f = { .Fun = {
                .retType = retType, 
                .paramTypes = static_cast<UsedType**>(std::malloc(sizeof(tp._f.Fun.paramTypes[0]) * paramTypes.size())), 
                .paramCount = paramTypes.size(), 
            }}}; 
            for (size_t i = 0; i < paramTypes.size(); ++i) {
                tp._f.Fun.paramTypes[i] = paramTypes[i];
            }
        
            UsedType* newPtr = new UsedType(tp);
            _typeSt.push_back(newPtr);
            return newPtr;
        }

        return *it;
    }

    
    void push_scope() {
        ex_assert(curScope);
        Scope* newScope = new Scope(curScope);
        curScope->subscopes.push_back(newScope);
        curScope = newScope;
        add_i(Instr {
            .tp = InstrType::SCOPE_START, 
            .arg1 = {.tp = InstrArgType::SCOPE_REF, .data = { .scope = curScope }}, 
            .arg2 = NanInstrarg, 
            .arg3 = NanInstrarg, 
            .size = -1 
        });
    }

    void pop_scope() {
        ex_assert(curScope->parentScope);
        curScope = curScope->parentScope;
        add_i(Instr {
            .tp = InstrType::SCOPE_END, 
            .arg1 = {.tp = InstrArgType::SCOPE_REF, .data = { .scope = curScope }}, 
            .arg2 = NanInstrarg, 
            .arg3 = NanInstrarg, .size = -1 
        });
    }

    StackLoc stack_alloc(size_t size) {
        curScope->stackOff += size;
        _instrs.push_back(Instr{ 
            .tp = InstrType::STACKALLOC, 
            .arg1 = { .tp = InstrArgType::STACK_LOC, .data = { .st = StackLoc { .scope = curScope, .stackOff = static_cast<int>(size), }}},
        });
        return StackLoc{.scope = curScope, .stackOff=int(curScope->stackOff-size)};
    }

    void add_param_i(ParamIndex p, StackLoc st) {
        _instrs.push_back(Instr{
            .tp = InstrType::GET_PARAM,
            .arg1 = {.tp = InstrArgType::PARAM, .data = { .param = p }},
            .arg2 = {.tp = InstrArgType::STACK_LOC, .data = { .st = st }},
            .arg3 = NanInstrarg,
            .size = -1,
        });
    }

    void add_i(Instr i) {
        _instrs.push_back(i);
    }

    void add_i(InstrType iType, int size, Reg src, Reg dst) {
        _instrs.push_back(Instr{
            .tp = iType,
            .arg1 = { .tp = InstrArgType::REG, .data = {.reg = src }},
            .arg2 = { .tp = InstrArgType::REG, .data = {.reg = dst }},
            .arg3 = NanInstrarg,
            .size = size,
        });
    }

    void add_i(InstrType iType, int size, Reg src, RegOff dst) {
        _instrs.push_back(Instr{
            .tp = iType,
            .arg1 = {.tp = InstrArgType::REG, .data = { .reg = src }},
            .arg2 = {.tp = InstrArgType::REG_OFF, .data = { .regOff = dst }},
            .arg3 = NanInstrarg,
            .size = size,
        });
    }

    void add_i(InstrType iType, int size, RegOff src, Reg dst) {
        _instrs.push_back(Instr{
            .tp = iType,
            .arg1 = {.tp = InstrArgType::REG_OFF, .data = { .regOff = src }},
            .arg2 = {.tp = InstrArgType::REG, .data = { .reg = dst }},
            .arg3 = NanInstrarg,
            .size = size,
        });
    }

    void add_i(InstrType iType, int size, Reg src, StackLoc dst) {
        _instrs.push_back(Instr{
            .tp = iType,
            .arg1 = {.tp = InstrArgType::REG, .data = { .reg = src }},
            .arg2 = {.tp = InstrArgType::STACK_LOC, .data = { .st = dst }},
            .arg3 = NanInstrarg,
            .size = size,
        });
    }

    void add_i(InstrType iType, int size, StackLoc src, Reg dst) {
        _instrs.push_back(Instr{
            .tp = iType,
            .arg1 = {.tp = InstrArgType::STACK_LOC, .data = { .st = src }},
            .arg2 = {.tp = InstrArgType::REG, .data = { .reg = dst }},
            .arg3 = NanInstrarg,
            .size = size,
        });
    }

    void add_i(InstrType iType, int size, Lit l, Reg dst) {
        _instrs.push_back(Instr{
            .tp = iType,
            .arg1 = {.tp = InstrArgType::LIT, .data = { .lit = l }},
            .arg2 = {.tp = InstrArgType::REG, .data = { .reg = dst }},
            .arg3 = NanInstrarg,
            .size = size,
        });
    }

    Mark add_mark() {
        Mark m = curScope->next_mark();
        _instrs.push_back(Instr{
            .tp = InstrType::MARK,
            .arg1 = {.tp = InstrArgType::MARK, .data = { .mark = m }},
            .arg2 = NanInstrarg,
            .arg3 = NanInstrarg,
            .size = -1,
        });
        return m;
    }

    void set_cursor(const Cursor *c) {
        ex_assert(c);
        _cur = c;
    }

    // UsedType* struct_type(std::string_view structName, std::vector<StructField> &&fields) {
    //     UsedType tp = { ._class=UsedTypeClass::Struct, ._f = { .Struct = {
    //         .structName = structName,
    //         .fields = static_cast<StructField*>(std::malloc(sizeof(tp._f.Struct.fields[0]) * fields.size())), 
    //         .fieldCount = fields.size(), 
    //     }}};
    //     for (size_t i = 0; i < fields.size(); ++i) {
    //         tp._f.Struct.fields[i] = fields[i];
    //     }

    //     auto it = std::find_if(_typeSt.begin(), _typeSt.end(), [&tp](const auto &arg) { return arg->is_same_inst(tp); });
    //     if (it == _typeSt.end()) {
    //         UsedType* newPtr = new UsedType(tp);
    //         _typeSt.push_back(newPtr);
    //         return newPtr;
    //     }

    //     free(tp.fields);
    //     return *it;
    // }


    ~Context() {
        for (auto &p: _typeSt) {
            if (p->_class == UsedTypeClass::Fun) {
                free(p->_f.Fun.paramTypes);
            }
            delete p;
            p = nullptr;
        }

        for (auto &sc: _scopeSt) {
            delete sc;
            sc = nullptr;
        }
    }
};


void ctx_set_var(Context &ctx, VarLoc var, bool *err) {
    auto curScope = ctx.curScope;
    ex_assert(curScope);
    if (std::find_if(curScope->vars.begin(), curScope->vars.end(), [&var](const auto &d){ return var.decl.varName == d.decl.varName; }) != curScope->vars.end()) {
        *err = true;
        ctx.set_err(std::string("Already had var with name [") + std::string(var.decl.varName) + "]", ctx._cur);
        return;
    }

    *err = false;
    curScope->vars.push_back(var);
}

VarLoc ctx_get_var(Context &ctx, std::string_view varName, bool *err) {
    auto curScope = ctx.curScope;
    auto it = std::find_if(curScope->vars.begin(), curScope->vars.end(), [&varName](const auto &d){ return varName == d.decl.varName; });
    if (it == curScope->vars.end()) {
        *err = true;
        ex_assert(false, "DEBUG");
        return {};
    }

    return *it;
}
    
void ctx_set_type(Context &ctx, UsedType* type, bool *err) {
    auto curScope = ctx.curScope;
    ex_assert(curScope);
    ex_assert(type->_class == UsedTypeClass::Type);
    if (std::find_if(curScope->types.begin(), curScope->types.end(), [&type](const auto &d) { return type->name() == d->name(); }) 
    != curScope->types.end()) {
        *err = true;
    }
    curScope->types.push_back(type);
}



#endif