

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

    UsedType* retType() {
        assert(_class == UsedTypeClass::Fun);
        return _f.Fun.retType;
    }

    std::vector<UsedType*> paramTypes() {
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

class Scope {
public:
    Scope* parentScope;
    std::vector<Scope*> subscopes;

    std::vector<VarDecl> vars;
    std::vector<UsedType*> types;

    Scope(Scope *parentScope) : parentScope{parentScope} {

    }

    Scope(const Scope& other) = delete;
    Scope(Scope&& other) = delete;

    Scope& operator=(const Scope& other) = delete;
    Scope& operator=(Scope&& other) = delete;
};

class Context {
public:

    std::vector<UsedType*> _typeSt; 
    std::vector<Scope*> _scopeSt; 

    Scope *curScope;

    Context() 
    : _typeSt{

        #define LOC(NAME)\
        new UsedType({ ._class = UsedTypeClass::Type, ._f = { .Type = {.name = NAME,  .isConst = false, .isStructKind = false, .fields = nullptr, .fieldCount = 0 } }})
        LOC("int"),
        LOC("void"),
        LOC("char"),
        #undef LOC
        
    } {
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

    void add_var(VarDecl decl, bool *exists) {
        curScope->vars.push_back(decl);
    }

    void add_type(UsedType* type, bool *exists) {
        assert(type->_class == UsedTypeClass::Type);
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


    void add_i(Instr instr) {

        
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


