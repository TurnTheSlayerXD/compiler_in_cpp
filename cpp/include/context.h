

enum class UsedTypeClass {
    Type,
    Ptr,
    Arr,
    Fun,
};
struct UsedType {
    UsedTypeClass _class;

    union {
        struct Type {
            std::string_view name;
            bool isConstType;
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
    
    bool is_same(const UsedType &rhs) {
        if (_class != rhs._class) {
            return false;
        }
        switch (_class) {
            case UsedTypeClass::Type: 
                return _f.Type.name == rhs._f.Type.name && _f.Type.isConstType == rhs._f.Type.isConstType;
            case UsedTypeClass::Ptr:
                return _f.Ptr.ptrTo == rhs._f.Ptr.ptrTo && _f.Ptr.isConstPtr == rhs._f.Ptr.isConstPtr;
            case UsedTypeClass::Fun: 
                if (_f.Fun.retType != rhs._f.Fun.retType) {
                    return false;
                }
                if (_f.Fun.paramCount != rhs._f.Fun.paramCount) {
                    return false;
                }
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




class Scope {

};

class Context {
public:

    std::vector<UsedType*> _typeSt; 

    Context() 
    : _typeSt{
        new UsedType({ ._class = UsedTypeClass::Type, ._f = { .Type = {.name = "int",  .isConstType = false} }}),
        new UsedType({ ._class = UsedTypeClass::Type, ._f = { .Type = {.name = "void", .isConstType = false} }}),
        new UsedType({ ._class = UsedTypeClass::Type, ._f = { .Type = {.name = "char", .isConstType = false} }}),
    } {
        

    }

    #define PAT\
        auto it = std::find_if(_typeSt.begin(), _typeSt.end(), [&tp](const auto &arg) { return arg->is_same(tp); }); \
        if (it == _typeSt.end()) { \
            UsedType* newPtr = new UsedType(tp); \
            _typeSt.push_back(newPtr); \
            return newPtr; \
        } \
        return *it;

    UsedType* type(std::string_view nm, bool isConst) {
        UsedType tp = { ._class = UsedTypeClass::Type, ._f = { .Type = { .name = nm, .isConstType = isConst}}}; 
        PAT
    }

    UsedType* ptr_type(UsedType *ptrTo, bool isConst) {
        UsedType tp = { ._class=UsedTypeClass::Ptr, ._f = { .Ptr = {.ptrTo = ptrTo, .isConstPtr = isConst}} }; 
        auto it = std::find_if(_typeSt.begin(), _typeSt.end(), [&tp](const auto &arg) { return arg->is_same(tp); });
        if (it == _typeSt.end()) {
            UsedType* newPtr = new UsedType(tp);
            _typeSt.push_back(newPtr);
            return newPtr;
        }

        return *it;
    }

    UsedType* fun_type(UsedType* retType, std::vector<UsedType*> &&paramTypes) {
        UsedType tp = { ._class=UsedTypeClass::Fun, ._f = { .Fun = {
            .retType = retType, 
            .paramTypes = static_cast<UsedType**>(std::malloc(sizeof(tp._f.Fun.paramTypes[0]) * paramTypes.size())), 
            .paramCount = paramTypes.size(), 
        }}}; 
        for (size_t i = 0; i < paramTypes.size(); ++i) {
            tp._f.Fun.paramTypes[i] = paramTypes[i];
        }

        auto it = std::find_if(_typeSt.begin(), _typeSt.end(), [&tp](const auto &arg) { return arg->is_same(tp); });
        if (it == _typeSt.end()) {
            UsedType* newPtr = new UsedType(tp);
            _typeSt.push_back(newPtr);
            return newPtr;
        }

        return *it;
    }

    ~Context() {
        for (auto &p: _typeSt) {
            if (p->_class == UsedTypeClass::Fun) {
                free(p->_f.Fun.paramTypes);
            }

            delete p;
            p = nullptr;
        }
    }
};


