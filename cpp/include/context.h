#ifndef CONTEXT_H
#define CONTEXT_H

#include <instructions.h>

enum class UsedTypeClass {
    Type,
    Ptr,
    Arr,
    Fun,
};


struct StructField;

struct UsedType {
    UsedTypeClass _class;

    union {
        struct Type {
            std::string_view name;
            bool isConst;

            bool isStructKind;
            StructField* fields;
            size_t fieldCount;
        } Type;

        struct Ptr {
            UsedType* ptrTo;
            bool isConstPtr;
        } Ptr;

        // struct Arr {
        //     UsedType* arrayOf;
        // } Arr;

        struct Fun {
            UsedType* retType;
            UsedType** paramTypes;
            size_t paramCount;
        } Fun;

    } _f;

    size_t type_size() {
        static_assert(false && "NOT IMPLEMENTED");
    }

    UsedType* ret_type() {
        assert(_class == UsedTypeClass::Fun);
        return _f.Fun.retType;
    }

    std::string_view name() {
        assert(_class == UsedTypeClass::Type);
        return _f.Type.name;
    }

    std::vector<UsedType*> param_types() {
        assert(_class == UsedTypeClass::Fun);
        std::vector<UsedType*> vec(_f.Fun.paramCount);
        for (size_t i = 0; i < _f.Fun.paramCount; ++i) {
            vec[i] = _f.Fun.paramTypes[i];
        }
        return vec;
    }
    
    bool is_same_inst(const UsedType &rhs) {
        if (_class != rhs._class) {
            return false;
        }
        switch (_class) {
            case UsedTypeClass::Type: 
                return _f.Type.name == rhs._f.Type.name && _f.Type.isConst == rhs._f.Type.isConst;
            case UsedTypeClass::Ptr:
                return _f.Ptr.ptrTo == rhs._f.Ptr.ptrTo && _f.Ptr.isConstPtr == rhs._f.Ptr.isConstPtr;
            case UsedTypeClass::Fun: 
                if (_f.Fun.retType != rhs._f.Fun.retType) return false;
                if (_f.Fun.paramCount != rhs._f.Fun.paramCount) return false;
                for (size_t i = 0; i < _f.Fun.paramCount; ++i) {
                    if (_f.Fun.paramTypes[i] != rhs._f.Fun.paramTypes[i]) 
                        return false;
                }
                return true;
            default: 
                assert(false && "UNREACHABLE");
                return false;
        }
    }

    bool operator==(const UsedType& rhs) = delete;
};

struct StructField {
    std::string_view fieldName;
    UsedType* fieldType;
};

struct VarDecl {
    std::string_view varName;
    UsedType *varType;
};

struct VarLoc {
    VarDecl decl;
    StackP stackP;
};

class Scope {
public:
    Scope* parentScope;
    std::vector<Scope*> subscopes;

    std::vector<VarLoc> vars;
    std::vector<UsedType*> types;

    Scope(Scope *parentScope) : parentScope{parentScope} {
        
    }

    Scope(const Scope& other) = delete;
    Scope(Scope&& other) = delete;

    Scope& operator=(const Scope& other) = delete;
    Scope& operator=(Scope&& other) = delete;
};

struct FunDefined {
    VarDecl funDecl;
    bool defined;
};

class Context {
public:

    std::vector<UsedType*> _typeSt; 
    std::vector<Scope*> _scopeSt; 

    std::vector<FunDefined> _funs;

    Scope *curScope;

    std::vector<std::string> _errs;

    Cursor* _cur;

    Context() 
    : _typeSt{

        #define LOC(NAME)\
        new UsedType({ ._class = UsedTypeClass::Type, ._f = { .Type = {.name = NAME,  .isConst = false, .isStructKind = false, .fields = nullptr, .fieldCount = 0 } }})
        LOC("int"),
        LOC("void"),
        LOC("char"),
        #undef LOC
        
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
            UsedType* newPtr = new UsedType({ ._class = UsedTypeClass::Type, ._f = { .Type = { .name = nm, .isConst = isConst, .isStructKind = false, .fields = nullptr, .fieldCount = 0 }}});
            _typeSt.push_back(newPtr);
            return newPtr;
        }
        return *it;
    }

    void add_err(std::string &&msg, Cursor* cur) {
        assert(cur);
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

    void add_var(VarLoc var, bool *err) {
        if (std::find_if(curScope->vars.begin(), curScope->vars.end(), [&var](const auto &d){ return var.decl.varName == d.decl.varName; }) != curScope->vars.end()) {
            *err = true;
            add_err(std::string("Already had var with name [") + std::string(var.decl.varName) + "]", _cur);
        }
        *err = false;
        curScope->vars.push_back(var);
    }

    void add_type(UsedType* type, bool *err) {
        assert(type->_class == UsedTypeClass::Type);
        if (std::find_if(curScope->types.begin(), curScope->types.end(), [&type](const auto &d) { return type->name() == d->name(); }) 
        != curScope->types.end()) {
            *err = true;
        }
        curScope->types.push_back(type);
    }

    void push_scope() {
        assert(curScope);
        Scope* newScope = new Scope(curScope);
        curScope->subscopes.push_back(newScope);
        curScope = newScope;
    }

    void pop_scope() {
        assert(curScope->parentScope);
        curScope = curScope->parentScope;
    }

    void set_ret_addr_p(StackP ) {
        static_assert(false);
    }

    StackP stack_alloc(size_t size) {
        static_assert(false);
    }

    void add_i(InstrType iType, Register reg, Register reg) {
        static_assert(false && "Not implemented");
    }

    void add_i(InstrType iType, Register reg, RegisterWithOffset arg) {
        static_assert(false && "Not implemented");
    }

    void add_i(InstrType iType, Register reg, StackP stP) {
        static_assert(false && "Not implemented");
    }

    void add_i(InstrType iType, RegisterWithOffset arg, Register reg) {
        static_assert(false && "Not implemented");
    }

    void add_i(InstrType iType, StackP stP, Register reg) {
        static_assert(false && "Not implemented");
    }

    void add_i(InstrType iType, InstrArg arg) {
        static_assert(false && "Not implemented");
    }


    void add_fun(VarDecl funDecl, bool withDefinition, bool *err) {
        assert(funDecl.varType->_class == UsedTypeClass::Fun);

        bool wasDeclared = false;
        for (auto &d: _funs) {
            if (d.funDecl.varName == funDecl.varName ) {
                bool isSameInst = d.funDecl.varType->is_same_inst(*funDecl.varType);
                if (isSameInst && d.defined && withDefinition) {
                    *err = true;
                    add_err(std::string("Function [") + std::string(funDecl.varName) + "] already defined", _cur);
                }
                else if (!isSameInst) {
                    add_err(std::string("Function [") + std::string(funDecl.varName) + "] had another prototype in previous declaration", _cur);
                }
                else {
                    d.defined = withDefinition;
                    wasDeclared = true;
                }
            }
        }

        if (!wasDeclared) {
            _funs.push_back(FunDefined{.funDecl = funDecl, .defined = withDefinition});
        }
    }

    set_cursor(Cursor *c) {
        assert(c);
        _cur = c;
    }

    Register get_free_reg() {
        static_assert(false && "NOT IMPLEMENTED");
    }


    template <class ...T>
    void take_registers(T... regs) {
        using F = std::tuple_element_t<0, std::tuple<T...>>;
        static_assert(std::is_same<F, Register>());
        static_assert(false && "NOT IMPLEMENTED");
    }

    template <class ...T>
    void free_registers(T... regs) {
        using F = std::tuple_element_t<0, std::tuple<T...>>;
        static_assert(std::is_same<F, Register>());
        static_assert(false && "NOT IMPLEMENTED");
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

#endif