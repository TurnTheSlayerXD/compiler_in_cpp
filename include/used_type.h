


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