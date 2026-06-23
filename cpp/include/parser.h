#ifndef PARSER_H
#define PARSER_H

#include <help.h>

class Parser;
class Expr;

using CExpr = const Expr;

class RefExpr;
class Term;
class Seq;
class Or;
class Opt;
class Any;
class OneOrMore;
class StructDecl;
class TypeUse;

template <class T>
struct PtrStorage {
    std::vector<T*> _ptrs;
    ~PtrStorage() {
        for (auto &p: _ptrs) {
            delete p;
            p = nullptr;
        }
    }
};

template <class T>
struct Ptr {
private:
    T* _ptr;
public:
    Ptr(): _ptr{nullptr} {}
    Ptr(T *ptr): _ptr{ptr} {}

    Ptr(const Ptr& rhs) = delete;
    Ptr(Ptr&& rhs) noexcept {
        _ptr = rhs._ptr;
        rhs._ptr = nullptr;
    }

    Ptr& operator=(const Ptr& rhs) = delete;
    
    Ptr& operator=(Ptr&& rhs) noexcept {
        Ptr garb(std::move(*this));
        _ptr = rhs._ptr;
        rhs._ptr = nullptr;
        return *this;    
    }


    bool is_null() { return _ptr == nullptr;}
    T* get_ptr(){ return _ptr;}  


    ~Ptr() {
        delete _ptr;
        _ptr = nullptr;
    }
};

struct NamedStorage {    
    struct ExprWithName {
        std::string_view name;
        CExpr *expr;
    };

    std::vector<ExprWithName> _items;
    
    void set_name(CExpr* expr, std::string_view name);
    
    CExpr* find(std::string_view name);

    std::string_view try_find_name(CExpr *CExpr);
};

class Parser {
public:
    PtrStorage<RefExpr> _refSt;
    PtrStorage<Term> _termSt;
    PtrStorage<Seq> _seqSt;
    PtrStorage<Or> _orSt;
    PtrStorage<Opt> _optSt;
    PtrStorage<Any> _anySt;
    PtrStorage<OneOrMore> _oneOrMoreSt;

    Ptr<StructDecl> _structdeclSt;
    Ptr<TypeUse> _typeuseSt;
    
    NamedStorage _namedStorage;

    std::vector<std::string_view> _decltypes;
    std::vector<std::string_view> _msgs;

    Parser(): _decltypes({"int", "char", "void"})  {}

    Parser(const Parser &rhs) = delete;
    Parser(Parser &&rhs) = delete;
    Parser& operator =(const Parser &rhs) = delete;
    Parser& operator =(Parser &&rhs) = delete;

    CExpr* named_ref(std::string_view exprName);

    CExpr* term(TokenType tokType);

    template <class ...T> CExpr* seq(NodeType nodeType, T ...subexprs);
    template <class ...T> CExpr* or_(T ...subexprs);
    template <class ...T> CExpr* opt(T ...subexprs);
    template <class ...T> CExpr* any(T ...subexprs);
    template <class ...T> CExpr* one_or_more(T ...subexprs);
    
    CExpr* structdecl();
    CExpr* typeuse();


    bool __recurs_check_cycles(CExpr* expr, std::unordered_set<CExpr*> &set, std::vector<std::string_view> &path);

    bool detect_cycles(CExpr* const e);

    bool has_decltype(std::string_view nm) {
        return std::find(_decltypes.begin(), _decltypes.end(), nm) != _decltypes.end();
    }

    void add_new_decltype(std::string_view nm) {
        assert(std::find(_decltypes.begin(), _decltypes.end(), nm) == _decltypes.end() && "Type already exist");
        _decltypes.push_back(nm);
    }

    void add_msg(std::string_view nm) {
        _msgs.push_back(nm);
    }

    bool __can_reach(CExpr *v, std::vector<std::string_view> &path);
};

void NamedStorage::set_name(CExpr* expr, std::string_view name) {
    auto pos = std::find_if(_items.begin(), _items.end(), [&name](const auto &arg){ return arg.name == name; });
    assert(pos == _items.end());
    _items.push_back( ExprWithName {.name = name, .expr = expr});
}

CExpr* NamedStorage::find(std::string_view name) {
    auto pos = std::find_if(_items.begin(), _items.end(), [&name](const auto &arg){ return arg.name == name; });
    assert(pos != _items.end());
    return pos->expr;
}

std::string_view NamedStorage::try_find_name(CExpr *expr) {
    for (auto &e: _items) {
        if (e.expr == expr) {
            return e.name;
        }
    }
    return "";
}

class Expr {
public:
    virtual Node* eval(Tokenizer &t) const = 0;
    virtual ~Expr(){}

    Parser& _p;
    Expr(Parser &p): _p{p} {
    }

    CExpr* set_name(std::string_view name) const {
        _p._namedStorage.set_name(this, name);
        return this;
    }
};

class RefExpr: public Expr {
public:
    std::string_view _exprName;
    RefExpr(Parser &p, std::string_view exprName): Expr(p), _exprName{exprName} {    }

    Node* eval(Tokenizer &t) const {
        auto expr = _p._namedStorage.find(_exprName);
        return expr->eval(t);
    }

    CExpr* get_real_expr() const {
        return _p._namedStorage.find(_exprName);
    }
};

class Term: public Expr {
public:
    NodeType _nodeType;
    TokenType _tokType;

    Term(Parser &p, TokenType tokType): Expr(p), _nodeType{NodeType::Leaf}, _tokType{tokType} {}

    Node* eval(Tokenizer &t) const override {
#define _EXIT(ARG) if (ARG) { return new Node(_nodeType, tok); } else { t.reset_pos(initPos); return nullptr; }
        auto initPos = t.get_pos();
        Token tok;
        if (t.eof()) {
            _EXIT(false);
        }
        tok = t.next_token();
        if (tok.type != _tokType) {
            _EXIT(false);
        }
        _EXIT(true);
#undef _EXIT
    }

};


template <size_t D, class ...U>
void _append(std::array<const Expr*, D> &dst, Parser &p, size_t index, const char *name, U ...rest) {
    dst[index] = p.named_ref(name);
    _append(dst, p, index+1, rest...);
}
template <size_t D, class ...U>
void _append(std::array<const Expr*, D> &dst, Parser &p, size_t index, CExpr *expr, U ...rest) {
    dst[index] = expr;
    _append(dst, p, index+1, rest...);
}
template <size_t D, class ...U>
void _append(std::array<const Expr*, D> &dst, Parser &p, size_t index, TokenType tokType, U ...rest) {
    dst[index] = p.term(tokType);
    _append(dst, p, index+1, rest...);
}
template <size_t D>
void _append(std::array<const Expr*, D> &dst, Parser &p, size_t index) {
    (void) dst, (void) p, (void) index;
}
template <class T>
bool _eq(const T& lhs, const T& rhs) {
    if (lhs.size() != rhs.size()) return false;
    for (size_t i = 0; i < lhs.size(); ++i) {
        if (lhs.at(i) != rhs.at(i)) return false;
    }
    return true;
}

class Seq: public Expr {
public:
    NodeType _nodeType;
    
    Seq(Parser &p, NodeType nodeType): Expr(p), _nodeType{nodeType} { }
    
    virtual CExpr* at(size_t index) const = 0;
    virtual size_t size() const = 0;
    
    bool eq(const Seq& rhs) const {
        return _eq(*this, rhs);
    }  
};

template <size_t N>
class TemplateSeq: public Seq {
public:
    std::array<CExpr*, N> _subexprs;
    
    TemplateSeq(Parser &p, NodeType nodeType): Seq(p, nodeType) { }

    Node* eval(Tokenizer &t) const override {
        Node v(_nodeType);
        auto initPos = t.get_pos();
        for (auto subexpr: _subexprs) {
            Node *res = subexpr->eval(t);
            if (!res) {
                t.reset_pos(initPos); return nullptr;
            }
            v.children.push_back(res);
        }
        return new Node(std::move(v));
    }
    
    CExpr* at(size_t index) const override {
        return _subexprs[index];
    }
    
    size_t size() const override {
        return N;
    }
    
};

class Or: public Expr {
public:
    Or(Parser &p): Expr(p){}
    virtual CExpr* at(size_t index) const = 0;
    virtual size_t size() const = 0;

    bool eq(const Or& rhs) const {
        return _eq(*this, rhs);
    }  
};

template <size_t N>
class TemplateOr: public Or {
public:
    std::array<CExpr*, N> _subexprs;
    TemplateOr(Parser &p): Or(p) {}
    
    Node* eval(Tokenizer &t) const override {
        auto initPos = t.get_pos();
        for (auto subexpr: _subexprs) {
            Node *res = subexpr->eval(t);
            if (res) {
                return res;
            }
            t.reset_pos(initPos);
        }
        return nullptr;
    }

    CExpr* at(size_t index) const override { return _subexprs[index]; }
    size_t size() const override { return N; }
};


class Opt: public Expr {
public:
    Opt(Parser &p): Expr(p){}
    virtual CExpr* at(size_t index) const = 0;
    virtual size_t size() const = 0;
    bool eq(const Opt& rhs) const { return _eq(*this, rhs); }
};

template <size_t N>
class TemplateOpt: public Opt {
public:
    std::array<CExpr*, N> _subexprs;

    TemplateOpt(Parser& p): Opt(p) { static_assert(N == 1); }

    CExpr* at(size_t index) const override { return _subexprs[index]; }
    size_t size() const override { return N; }
    
    Node* eval(Tokenizer &t) const override {
        auto initPos = t.get_pos();
        Node *v = _subexprs[0]->eval(t);
        if (v) {
            return v;
        }
        t.reset_pos(initPos);
        return node_plug();
    }
};

class Any: public Expr {
public:
    Any(Parser &p): Expr(p){}
    virtual CExpr* at(size_t index) const = 0;
    virtual size_t size() const = 0;
    bool eq(const Any& rhs) const { return _eq(*this, rhs); }
};

template <size_t N>
class TemplateAny: public Any {
public:
    std::array<CExpr*, N> _subexprs;
    TemplateAny(Parser& p): Any(p) { static_assert(N == 1); }

    Node* eval(Tokenizer &t) const override {
        auto pos = t.get_pos();
        Node newV(NodeType::Any);
        auto subexpr = _subexprs[0];
        int count = 0;
        while(true) {
            if (t.eof()) {
                break;
            }
            auto v = subexpr->eval(t);
            if (v) {
                newV.children.push_back(v);
                pos = t.get_pos();
            }
            else {
                t.reset_pos(pos);
                break;
            }

            count++;
            if (count > 500) {
                assert(false && "INFINITE LOOP");
            }
        }

        return new Node(std::move(newV));
    }
    
    CExpr* at(size_t index) const override { return _subexprs[index]; }
    size_t size() const override { return N; }
};

class OneOrMore: public Expr {
public:
    OneOrMore(Parser &p): Expr(p){}
    virtual CExpr* at(size_t index) const = 0;
    virtual size_t size() const = 0;
    bool eq(const OneOrMore& rhs) const { return _eq(*this, rhs); }
};

template <size_t N>
class TemplateOneOrMore: public OneOrMore {
public:
    std::array<CExpr*, N> _subexprs;
    TemplateOneOrMore(Parser& p): OneOrMore(p) { static_assert(N == 1); }
    
    Node* eval(Tokenizer &t) const override {
        auto pos = t.get_pos();
        auto subexpr = _subexprs[0];
        if (t.eof()) {
            return nullptr;
        }
        auto v = subexpr->eval(t);
        if (!v) {
            t.reset_pos(pos);
            return nullptr;
        }
        pos = t.get_pos();
        Node newV(NodeType::OneOrMore);
        newV.children.push_back(v);


        int count = 0;
        while(true) {
            if (t.eof()) {
                break;
            }
            auto v = subexpr->eval(t);
            if (v) {
                newV.children.push_back(v);
                pos = t.get_pos();
            }
            else {
                t.reset_pos(pos);
                break;
            }

            count++;

            if (count > 500) {
                std::cout << "Typename of subexpr with INF LOOP" << ": [" << typeid(*_subexprs[0]).name() << "]" << std::endl;
                std::cout << "Typename of subexpr with INF LOOP" << ": [" << newV.children[0]->type << "]" << std::endl;
                assert(false && "INFINITE LOOP");
            }
        }

        return new Node(std::move(newV));
    }

    CExpr* at(size_t index) const override { return _subexprs[index]; }
    size_t size() const override { return N; }
};

class TypeUse: public Expr {
public:
    TypeUse(Parser &p): Expr(p) {}
    Node* eval(Tokenizer &t) const override {
#define _EXIT(ARG) if (ARG) { return new Node(NodeType::Leaf, tok); } else { t.reset_pos(initPos); return nullptr; }
        auto initPos = t.get_pos();
        Token tok;
        if (t.eof()) {
            _EXIT(false);
        }
        tok = t.next_token();
        if (tok.type != TokenType::WORD || !_p.has_decltype(tok.text)) {
            _EXIT(false);
        }

        _EXIT(true);
#undef _EXIT
    }
};

class StructDecl: public Expr {
public:
    StructDecl(Parser &p): Expr(p) {}
    Node* eval(Tokenizer &t) const override {
#define _EXIT(ARG) if (ARG) { return new Node(NodeType::Leaf, tok); } else { t.reset_pos(initPos); return nullptr; }
        auto initPos = t.get_pos();
        Token tok;
        if (t.eof()) {
            _EXIT(false);
        }
        tok = t.next_token();
        if (tok.type != TokenType::WORD) {
            _EXIT(false);
        }
        if (_p.has_decltype(tok.text)) {
            _EXIT(false);
        }

        _p.add_new_decltype(tok.text);
        _EXIT(true);
#undef _EXIT
    }
};


CExpr* Parser::named_ref(std::string_view exprName) {
    auto pos = std::find_if(_refSt._ptrs.begin(), _refSt._ptrs.end(), [&exprName](const auto &p) { return p->_exprName == exprName; });
    if (pos != _refSt._ptrs.end()) return *pos;
    RefExpr* newRef = new RefExpr(*this, exprName);
    _refSt._ptrs.push_back(newRef);
    return newRef;
}
    
CExpr* Parser::term(TokenType tokType) {
    auto pos = std::find_if(_termSt._ptrs.begin(), _termSt._ptrs.end(), [&tokType](const auto &p) { return p->_tokType == tokType; });
    if (pos != _termSt._ptrs.end()) return *pos;
    Term* newTerm = new Term(*this, tokType);
    _termSt._ptrs.push_back(newTerm);
    return newTerm;
}

template <class ...T>
CExpr* Parser::seq(NodeType nodeType, T ...subexprs) {
    TemplateSeq<sizeof...(subexprs)> obj(*this, nodeType);
    _append(obj._subexprs, *this, 0, subexprs...);
    auto pos = std::find_if(_seqSt._ptrs.begin(), _seqSt._ptrs.end(), [&obj](const auto &p) { return obj.eq(*p); });
    if (pos != _seqSt._ptrs.end()) return *pos;
    Seq* newPtr = new TemplateSeq<sizeof...(subexprs)>(obj);
    _seqSt._ptrs.push_back(newPtr);
    return newPtr;
}


#define PREPR_CR(FUN, TEMPL, STOR, TYPE)\
    template <class ...T> CExpr* Parser::FUN(T ...subexprs) { \
        TEMPL<sizeof...(subexprs)> obj(*this); \
        _append(obj._subexprs, *this, 0, subexprs...); \
        auto pos = std::find_if(STOR._ptrs.begin(), STOR._ptrs.end(), [&obj](const auto &p) { return obj.eq(*p); }); \
        if (pos != STOR._ptrs.end()) return *pos; \
        TYPE* newPtr = new TEMPL<sizeof...(subexprs)>(obj); \
        STOR._ptrs.push_back(newPtr); \
        return newPtr; \
    }

PREPR_CR(or_, TemplateOr, _orSt, Or)
PREPR_CR(opt, TemplateOpt, _optSt, Opt)
PREPR_CR(any, TemplateAny, _anySt, Any)
PREPR_CR(one_or_more, TemplateOneOrMore, _oneOrMoreSt, OneOrMore)

#undef PREPR_CR


CExpr* Parser::typeuse() {
    if (!_typeuseSt.is_null()) return _typeuseSt.get_ptr();
    _typeuseSt = Ptr<TypeUse>(new TypeUse(*this));
    return _typeuseSt.get_ptr();
}

CExpr* Parser::structdecl() {
    if (!_structdeclSt.is_null()) return _structdeclSt.get_ptr();
    _structdeclSt = Ptr<StructDecl>(new StructDecl(*this));
    return _structdeclSt.get_ptr();
}

bool Parser::__recurs_check_cycles(CExpr* expr, std::unordered_set<CExpr*> &set, std::vector<std::string_view> &path) {

    if (!expr) {
        assert(false && "Faced nullptr!");
        return false;
    }

    if (set.contains(expr)) {
        path.push_back(_namedStorage.try_find_name(expr));
        return true;
    }

    const TypeUse *typeuse = dynamic_cast<const TypeUse*>(expr);
    if (typeuse) {
        std::cout << "Typeuse found" << std::endl;
        return false;
    }

    const Seq *seq = dynamic_cast<const Seq*>(expr); 
    if (seq) {
        set.insert(expr);
        if (seq->size() == 0) {
            assert(false && "Faced seq of size = 0"); 
        } 
        CExpr *child = (seq)->at(0); 
        bool isCycleInChild = __recurs_check_cycles(child, set, path); 
        if (isCycleInChild) { 
            path.push_back(_namedStorage.try_find_name(expr)); 
        } 
        set.erase(expr); 
        return isCycleInChild; 
    } 
    
#define _PREPR_SHORTCUT(TYPE, VAR) \
        const TYPE *VAR = dynamic_cast<const TYPE*>(expr); \
        if (VAR) { \
            set.insert(expr); \
            for (size_t i = 0; i < VAR->size(); ++i) { \
                CExpr *child = VAR->at(i); \
                bool isCycleInChild = __recurs_check_cycles(child, set, path);  \
                if (isCycleInChild) { \
                    path.push_back(_namedStorage.try_find_name(expr)); \
                    return true; \
                } \
            } \
            set.erase(expr); \
            return false; \
        }

    _PREPR_SHORTCUT(Or, or_)
    _PREPR_SHORTCUT(Opt, opt)
    _PREPR_SHORTCUT(Any, any)
    _PREPR_SHORTCUT(OneOrMore, oneOrMore)

#undef _PREPR_SHORTCUT

    const Term* term = dynamic_cast<const Term*>(expr);
    if (term) {
        return false;
    }

    const RefExpr* ref = dynamic_cast<const RefExpr*>(expr);
    if (ref) {
        set.insert(expr);

        bool isCycleInChild = __recurs_check_cycles(ref->get_real_expr(), set, path);
        if (isCycleInChild) {
            path.push_back(ref->_exprName);
        }

        set.erase(expr);
        return isCycleInChild;
    }

    std::cout << "Typename of expr that couldn't be casted" << ": [" << typeid(*expr).name() << "]" << std::endl;

    assert(false && "UNREACHABLE");
    return false;
} 

bool Parser::detect_cycles(CExpr* e) {
    std::unordered_set<CExpr*> set;
    std::vector<std::string_view> path;
    bool isCycleInChild = __recurs_check_cycles(e, set, path);
    if (isCycleInChild) {
        for (size_t i = 0; i < path.size(); ++i) {
            std::cout<<"["<<i<<"]"<<":  "<<path[i]<<std::endl;
        }
        return true;
    }

    std::vector<CExpr*> stack = {e};
    std::unordered_set<CExpr*> unique_nodes;

    while (!stack.empty()) {
        auto v = stack.back();
        stack.pop_back();

        unique_nodes.insert(v);

        #define REP \
        if(ptr) { \
            for (size_t i = 0; i < ptr->size(); ++i) { \
                CExpr *child = ptr->at(i); \
                if (!unique_nodes.contains(child)) { \
                    stack.push_back(child); \
                } \
            } \
        }

        {const Or* ptr = dynamic_cast<const Or*>(v); REP}
        {const Opt* ptr = dynamic_cast<const Opt*>(v); REP}
        {const Any* ptr = dynamic_cast<const Any*>(v); REP}
        {const OneOrMore* ptr = dynamic_cast<const OneOrMore*>(v); REP}
        {const Seq* ptr = dynamic_cast<const Seq*>(v); REP}

        #undef REP
    }

    for (auto n: unique_nodes) {
        std::unordered_set<CExpr*> set;
        std::vector<std::string_view> path;
        bool isCycleInChild = __recurs_check_cycles(n, set, path);
        if (isCycleInChild) {
            for (size_t i = 0; i < path.size(); ++i) {
                std::cout<<"["<<i<<"]"<<":  "<<path[i]<<std::endl;
            }
            return true;
        }
    }

    for (auto n: unique_nodes) {
        {
            const Any* ptr = dynamic_cast<const Any*>(n);
            std::vector<std::string_view> path;
            if (ptr && __can_reach(ptr->at(0), path)) {
                std::cout << "Nondetermenistic Any seq detected from expr [" << _namedStorage.try_find_name(n) << "]" << std::endl;
                std::cout << "___________________________________________\n";
                for (auto p: path) {
                    std::cout << p << ", ";
                }
                std::cout << "\n___________________________________________\n";
                // return true;
            }
        }
        {
            const OneOrMore* ptr = dynamic_cast<const OneOrMore*>(n);
            std::vector<std::string_view> path;
            if (ptr && __can_reach(ptr->at(0), path)) {
                std::cout << "Nondetermenistic OneOrMore seq detected from expr [" << _namedStorage.try_find_name(n) << "]" << std::endl;
                std::cout << "___________________________________________\n";
                for (auto p: path) {
                    std::cout << p << ", ";
                }
                std::cout << "\n___________________________________________\n";
                // return true;
            }
        }
    }

    return false;
}

bool Parser::__can_reach(const CExpr *v, std::vector<std::string_view> &path) {
    // {
    //     const Opt* ptr = dynamic_cast<const Opt*>(v); 
    //     if (ptr) {
    //         path.push_back(_namedStorage.try_find_name(v));
    //         return true;
    //     }
    // }
    {
        const Any* ptr = dynamic_cast<const Any*>(v); 
        if (ptr) {
            path.push_back(_namedStorage.try_find_name(v));
            return true;
        }
    }
    {
        const Or* ptr = dynamic_cast<const Or*>(v); 
        if (ptr) {
            for (size_t i=0; i<ptr->size(); ++i) {
                if (__can_reach(ptr->at(i), path)) {
                    path.push_back(_namedStorage.try_find_name(ptr));
                    return true;
                }
            }
            return false;
        }
    }
    {
        const Seq* ptr = dynamic_cast<const Seq*>(v); 
        if (ptr){
            bool can = __can_reach(ptr->at(0), path);
            if (can) {
                path.push_back(_namedStorage.try_find_name(ptr));
                return true;
            }
            return false;
        }
    }
    {
        const OneOrMore* ptr = dynamic_cast<const OneOrMore*>(v); 
        if (ptr) {
            bool can = __can_reach(ptr->at(0), path);
            if (can) {
                path.push_back(_namedStorage.try_find_name(ptr));
                return true;
            }
            return false;
        }
    }

    return false;
}


#endif