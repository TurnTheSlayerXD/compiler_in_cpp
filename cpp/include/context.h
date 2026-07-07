#ifndef CONTEXT_H
#define CONTEXT_H

#include <instructions.h>

enum class UsedTypeClass {
    Type,
    Ptr,
    Arr,
    Fun,
};


struct UsedType;

struct StructField {
    std::string_view fieldName;
    UsedType* fieldType;
};

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


    std::string_view name() {
        assert(_class == UsedTypeClass::Type);
        return _f.Type.name;
    }

    bool is_struct_kind() {
        assert(_class == UsedTypeClass::Type);
        return _f.Type.isStructKind;
    }

    StructField* fields() {
        assert(_class == UsedTypeClass::Type && _f.Type.isStructKind);
        return _f.Type.fields;
    }
    size_t field_count() {
        assert(_class == UsedTypeClass::Type && _f.Type.isStructKind);
        return _f.Type.fieldCount;
    }

    UsedType* ret_type() {
        assert(_class == UsedTypeClass::Fun);
        return _f.Fun.retType;
    }

    UsedType** param_types() {
        assert(_class == UsedTypeClass::Fun);
        return _f.Fun.paramTypes;
    }
    size_t param_count() {
        assert(_class == UsedTypeClass::Fun);
        return _f.Fun.paramCount;
    }

    size_t type_size() {
        switch(_class) {
            case UsedTypeClass::Type:
                if (is_struct_kind()) {
                    size_t s = 0;
                    for (size_t i = 0; i < field_count(); ++i) {
                        s += fields()[i].fieldType->type_size();
                    }
                    return s;
                }
                if (name() == "int") {
                    return 4;
                } 
                if (name() == "char") {
                    return 1;
                }
                if (name() == "void") {
                    assert(false && "void does not have size!");
                }
                assert(false && "UNREACHABLE");
                return 0;
            case UsedTypeClass::Ptr:
                return PTR_SIZE;
            case UsedTypeClass::Fun:
                return PTR_SIZE;
            default:
                assert(false && "UNREACHABLE");
                return 0;
        }
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

static int _staticScopeCounter = 1;
class Scope {
    std::vector<std::array<char, 20>> _markHeap;

public:
    int id;
    int markCounter;
    int stackOff;

    Scope* parentScope;
    std::vector<Scope*> subscopes;

    std::vector<VarLoc> vars;
    std::vector<FunLoc> funs;

    std::vector<UsedType*> types;

    Scope(Scope *parentScope): id{_staticScopeCounter++}, markCounter{0}, stackOff{0}, parentScope{parentScope} {
    }

    Mark next_mark() {
        _markHeap.push_back({});
        auto &s = _markHeap.back();
        std::snprintf(s.data(), s.size(), "sc_%d_m_%d", id, markCounter++);
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
            UsedType* newPtr = new UsedType({ ._class = UsedTypeClass::Type, ._f = { .Type = { .name = nm, .isConst = isConst, .isStructKind = false, .fields = nullptr, .fieldCount = 0 }}});
            _typeSt.push_back(newPtr);
            return newPtr;
        }
        return *it;
    }

    void set_err(std::string &&msg, const Cursor* cur) {
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

    StackLoc stack_alloc(size_t size) {
        curScope->stackOff += size;
        return StackLoc{.scopeId = curScope->id, .stackOff=int(curScope->stackOff-size), .nm = ""};
    }

    StackLoc stack_alloc(size_t size, std::string_view nameToInclude) {
        curScope->stackOff += size;
        return StackLoc{.scopeId = curScope->id, .stackOff=int(curScope->stackOff-size), .nm = nameToInclude};
    }

    void add_param_i(ParamIndex p, StackLoc st) {
        _instrs.push_back(Instr{
            .tp = InstrType::PUT_PARAM,
            .arg1 = {.tp = InstrArgType::PARAM, .data = { .paramIndex = p }},
            .arg2 = {.tp = InstrArgType::STACK_LOC, .data = { .st = st }},
            .arg3 = NanInstrArgType,

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
            .arg3 = NanInstrArgType,
            .size = size,
        });
    }

    void add_i(InstrType iType, int size, Reg src, RegOff dst) {
        _instrs.push_back(Instr{
            .tp = iType,
            .arg1 = {.tp = InstrArgType::REG, .data = { .reg = src }},
            .arg2 = {.tp = InstrArgType::REG_OFF, .data = { .regOff = dst }},
            .arg3 = NanInstrArgType,
            .size = size,
        });
    }

    void add_i(InstrType iType, int size, RegOff src, Reg dst) {
        _instrs.push_back(Instr{
            .tp = iType,
            .arg1 = {.tp = InstrArgType::REG_OFF, .data = { .regOff = src }},
            .arg2 = {.tp = InstrArgType::REG, .data = { .reg = dst }},
            .arg3 = NanInstrArgType,
            .size = size,
        });
    }

    void add_i(InstrType iType, int size, Reg src, StackLoc dst) {
        _instrs.push_back(Instr{
            .tp = iType,
            .arg1 = {.tp = InstrArgType::REG, .data = { .reg = src }},
            .arg2 = {.tp = InstrArgType::STACK_LOC, .data = { .st = dst }},
            .arg3 = NanInstrArgType,
            .size = size,
        });
    }

    void add_i(InstrType iType, int size, StackLoc src, Reg dst) {
        _instrs.push_back(Instr{
            .tp = iType,
            .arg1 = {.tp = InstrArgType::STACK_LOC, .data = { .st = src }},
            .arg2 = {.tp = InstrArgType::REG, .data = { .reg = dst }},
            .arg3 = NanInstrArgType,
            .size = size,
        });
    }

    Mark add_mark() {
        Mark m = curScope->next_mark();
        _instrs.push_back(Instr {
            .tp = InstrType::MARK,
            .arg1 = {.tp = InstrArgType::MARK, .data = { .mark = m }},
            .arg2 = NanInstrArgType,
            .arg3 = NanInstrArgType,
            .size = -1,
        });
        return m;
    }

    void set_cursor(const Cursor *c) {
        assert(c);
        _cur = c;
    }

    Reg get_free_reg() {
        // static_assert(false && "NOT IMPLEMENTED");
    }

    template <class ...T>
    void free_registers(T... regs) {
        using F = std::tuple_element_t<0, std::tuple<T...>>;
        static_assert(std::is_same<F, Reg>());
        // static_assert(false && "NOT IMPLEMENTED");
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
    assert(curScope);
    if (std::find_if(curScope->vars.begin(), curScope->vars.end(), [&var](const auto &d){ return var.decl.varName == d.decl.varName; }) != curScope->vars.end()) {
        *err = true;
        ctx.set_err(std::string("Already had var with name [") + std::string(var.decl.varName) + "]", ctx._cur);
        return;
    }

    *err = false;
    curScope->vars.push_back(var);
}

void ctx_set_type(Context &ctx, UsedType* type, bool *err) {
    auto curScope = ctx.curScope;
    assert(curScope);
    assert(type->_class == UsedTypeClass::Type);
    if (std::find_if(curScope->types.begin(), curScope->types.end(), [&type](const auto &d) { return type->name() == d->name(); }) 
    != curScope->types.end()) {
        *err = true;
    }
    curScope->types.push_back(type);
}



#endif