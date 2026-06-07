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

template <class T>
struct PtrStorage {
    std::vector<T*> _ptrs;
    ~PtrStorage() {
        for (auto p: _ptrs) {
            delete p;
        }
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

    std::unique_ptr<StructDecl> _structdeclSt;
    std::unique_ptr<TypeUse> _typeuseSt;
    
    NamedStorage _namedStorage;

    Parser() {}

    Parser(const Parser &rhs) = delete;
    Parser(Parser &&rhs) = delete;
    Parser& operator =(const Parser &rhs) = delete;
    Parser& operator =(Parser &&rhs) = delete;

    CExpr* named_ref(std::string_view exprName);

    CExpr* term(TokenType tokType);

    template <class ...T>
    CExpr* seq(NodeType nodeType, T ...subexprs);

    template <class ...T>
    CExpr* or_(T ...subexprs);

    bool __recurs_check_cycles(CExpr* expr, CExpr* const par, std::unordered_set<CExpr*> &set, std::vector<std::string_view> &path);

    bool detect_cycles(CExpr* const e);
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
        Node *v = new Node(_nodeType);
        bool ok = true;
        auto initPos = t.get_pos();
        Destruct onDestruct([&ok, &v, &t, &initPos]{ if (!ok) { t.reset_pos(initPos); delete v; } });        

        if (t.eof()) {
            ok = false;
            return nullptr;
        }

        auto tok = t.next_token();
        if (tok.type != _tokType) {
            ok = false;
            return nullptr;
        }

        return new Node(_nodeType, tok); 
    }

};

class Seq: public Expr {
public:
    NodeType _nodeType;
    
    Seq(Parser &p, NodeType nodeType): Expr(p), _nodeType{nodeType}{}
    
    virtual CExpr* at(size_t index) const = 0;
    
    virtual size_t size() const = 0;
    
    bool eq(const Seq& rhs) const {
        if (size() != rhs.size()) {
            return false;
        }
        for (size_t i = 0; i < size(); ++i) {
            if (at(i) != rhs.at(i)) {
                return false;
            }
        }
        return true;
    }  
};

template <class ...T>
class TemplateSeq: public Seq {
public:
    std::array<CExpr*, sizeof...(T)> _subexprs;
    
    size_t _index; 
    
    TemplateSeq(Parser &p, NodeType nodeType, T ...subexprs): Seq(p, nodeType), _index{0} {
        append(subexprs...);
    }
    
    Node* eval(Tokenizer &t) const override {
        Node *v = new Node(_nodeType);
        bool ok = true;
        auto initPos = t.get_pos();
        Destruct onDestruct([&ok, &v, &t, &initPos]{ if (!ok) { t.reset_pos(initPos); delete v; } });        
        for (auto subexpr: _subexprs) {
            Node *res = subexpr->eval(t);
            if (!res) {
                ok = false;
                return nullptr;
            }
            v->children.push_back(res);
        }
        return v;
    }
    
    CExpr* at(size_t index) const override {
        return _subexprs[index];
    }
    
    size_t size() const override {
        return sizeof...(T);
    }
    
    template <class ...U>
    void append(const char *name, U ...rest) {
        _subexprs[_index++] = _p.named_ref(name);
        append(rest...);
    }
    
    template <class ...U>
    void append(CExpr *expr, U ...rest) {
        _subexprs[_index++] = expr;
        append(rest...);
    }
    
    template <class ...U>
    void append(TokenType tokType, U ...rest) {
        _subexprs[_index++] = _p.term(tokType);
        append(rest...);
    }
    
    void append() {
    }
};

template <class T>
bool _eq(const T& lhs, const T& rhs) {
    if (lhs.size() != rhs.size()) return false;
    for (size_t = 0; i < lhs.size(); ++i) {
        if (lhs.at(i) != rhs.at(i)) return false;
    }
    return true;
} 


class Or: public Expr {
public:

    Or(Parser &p): Expr(p){}

    virtual CExpr* at(size_t index) const = 0;

    virtual size_t size() const = 0;

    bool eq(const Or& rhs) const {
        if (size() != rhs.size()) {
            return false;
        }
        for (size_t i = 0; i < size(); ++i) {
            if (at(i) != rhs.at(i)) {
                return false;
            }
        }
        return true;
    }  
};

template <class ...T>
class TemplateOr: public Or {
public:

    std::array<CExpr*, sizeof...(T)> _subexprs;
    
    size_t _index; 
    
    TemplateOr(Parser &p, T ...subexprs): Or(p), _index{0} {
        append(subexprs...);
    }
    
    Node* eval(Tokenizer &t) const override {
        bool ok = true;
        auto initPos = t.get_pos();
        Destruct onDestruct([&ok, &t, &initPos]{ if (!ok) { t.reset_pos(initPos); }  });        
        for (auto subexpr: _subexprs) {
            Node *res = subexpr->eval(t);
            if (res) {
                return res;
            }
            t.reset_pos(initPos);
        }
        ok = false;
        return nullptr;
    }

    CExpr* at(size_t index) const override {
        return _subexprs[index];
    }

    size_t size() const override {
        return sizeof...(T);
    }
    
    template <class ...U>
    void append(const char *name, U ...rest) {
        _subexprs[_index++] = _p.named_ref(name);
        append(rest...);
    }

    template <class ...U>
    void append(CExpr *expr, U ...rest) {
        _subexprs[_index++] = expr;
        append(rest...);
    }

    template <class ...U>
    void append(TokenType tokType, U ...rest) {
        _subexprs[_index++] = _p.term(tokType);
        append(rest...);
    }

    void append() {
    }
};

template <size_t D, class ...U>
void _append(std::array<const Expr*, D> &dst, Parser &p, size_t index, const char *name, U ...rest) {
    dst[_index++] = p.named_ref(name);
    _append(dst, p, index+1, rest...);
}

template <size_t D, class ...U>
void _append(std::array<const Expr*, D> &dst, Parser &p, size_t index, CExpr *expr, U ...rest) {
    dst[_index++] = expr;
    _append(dst, p, index+1, rest...);
}

template <size_t D, class ...U>
void _append(std::array<const Expr*, D> &dst, Parser &p, size_t index, TokenType tokType, U ...rest) {
    dst[_index++] = p.term(tokType);
    _append(dst, p, index+1, rest...);
}

class Opt: public Expr {
public:
    Opt(Parser &p): Expr(p){}
    virtual CExpr* at(size_t index) const = 0;
    virtual size_t size(size_t index) const = 0;
    virtual bool eq(const Opt& rhs) const = 0;
};

template <class ...T>
class OptTemplate: public Opt {
public;
    std::array<CExpr*, 1> _subexprs;

    Opt(Parser &p, T ...subexprs): Expr(p) {
        static_assert(sizeof...(T) == 1);
        _append(_subexprs, p, 0, subexprs...);
    }
    
    Node* eval(Tokenizer &t) const override {
#define _EXIT_EVAL(ARG) if (ARG) { return v; } else { t.reset_pos(initPos); delete v; return nullptr; }
        auto initPos = t.get_pos();
        Node *v = _subexprs[0]->eval(t);
        if (v) {
            _EXIT_EVAL(true);
        }
        _EXIT_EVAL(false);
    }
};


template <class ...T>
class StructDecl: public Expr {
public:
    std::array<CExpr*, sizeof...(T)> _subexprs;
    size_t _index;
    
    StructDecl(Parser &p, T ...subexprs): Expr(p) {
        _append(subexprs, subexprs...);
    }

    Node* eval(Tokenizer &t) const override {
#define _EXIT(ARG) if (ARG) { return new Node(std::move(v)); } else { t.reset_pos(initPos); return nullptr; }
        Node v(NodeType::Struct);
        auto initPos = t.get_pos();
        for (auto subexpr: _subexprs) {
            Node *res = subexpr->eval(t);
            if (!res) {
                _EXIT(false);
            }
            v.children.push_back(res);
        }
        if (v.children.size() < 2 || v.children[1]->type != NodeType::Leaf || v.children[1]->get_tok().type != TokenType::WORD) {
            p.add_msg("Error in class StructDecl: Expected second token of struct declatration to be TokenType::WORD");
            _EXIT(false);
        }

        auto nodeStructName = v.children[1];
        if (this->_p.has_decltype(nodeStructName->get_tok().text)) {
            p.add_msg("Error in class StructDet: Redeclaration of type `", nodeStructName->get_tok().text, "`");
            _EXIT(false);
        }

        this->_p.add_new_decltype(nodeStructName->get_tok().text);
        _EXIT(true);
#undef _EXIT
    }
};

template <class ...T>
class TypeUse: public Expr {
public:
    std::array<Expr*, sizeof...(T)>
    TypeUse(Parser &p, T ...subexprs): Expr(p) {
        _append(subexprs...);
    }
    Node* eval(Tokenizer &t) const override {
#define _EXIT(ARG) if (ARG) { return new Node(std::move(v)); } else { t.reset_pos(initPos); return nullptr; }
        Node v(this->_nodeType);
        auto initPos = t.get_pos();
        for (auto subexpr: _subexprs) {
            Node *res = subexpr->eval(t);
            if (!res) {
                _EXIT(false);
            }
            v.children.push_back(res);
        }
        if(v.children.size() < 1 || v.children[0]->type != NodeType::Leaf || v.children[0]->get_tok().type != TokenType::WORD) {
            _EXIT(false);
        }
        auto nodeTypeName = v.children[0];
        if (!this->_p.has_decltype(nodeTypeName->get_tok().text)) {
            _EXIT(false);
        }
        _EXIT(true);
#undef _EXIT
    }
}


CExpr* Parser::named_ref(std::string_view exprName) {
    auto pos = std::find_if(_refSt._ptrs.begin(), _refSt._ptrs.end(), [&exprName](const auto &p) {
        return p->_exprName == exprName;
    });
    if (pos != _refSt._ptrs.end()) {
        return *pos;
    }
    RefExpr* newRef = new RefExpr(*this, exprName);
    _refSt._ptrs.push_back(newRef);
    return newRef;
}
    
CExpr* Parser::term(TokenType tokType) {
    auto pos = std::find_if(_termSt._ptrs.begin(), _termSt._ptrs.end(), [&tokType](const auto &p) {
        return p->_tokType == tokType;
    });
    if (pos != _termSt._ptrs.end()) {
        return *pos;
    }
    Term* newTerm = new Term(*this, tokType);
    _termSt._ptrs.push_back(newTerm);
    return newTerm;
}

template <class ...T>
CExpr* Parser::seq(NodeType nodeType, T ...subexprs) {
    TemplateSeq<T...> obj(*this, nodeType, subexprs...);
    auto pos = std::find_if(_seqSt._ptrs.begin(), _seqSt._ptrs.end(), [&obj](const auto &p) { 
        return obj.eq(*p);
    });
    if (pos != _seqSt._ptrs.end()) {
        return *pos;
    }
    Seq* newPtr = new TemplateSeq<T...>(obj);
    _seqSt._ptrs.push_back(newPtr);
    return newPtr;
}

template <class ...T>
CExpr* Parser::or_(T ...subexprs) {
    TemplateOr<T...> obj(*this, subexprs...);
    auto pos = std::find_if(_orSt._ptrs.begin(), _orSt._ptrs.end(), [&obj](const auto &p) { 
        return obj.eq(*p);
    });
    if (pos != _orSt._ptrs.end()) {
        return *pos;
    }
    Or* newPtr = new TemplateOr<T...>(obj);
    _orSt._ptrs.push_back(newPtr);
    return newPtr;
}

template <class ...T>
CExpr* Parser::typeuse(T ...subexprs) {
    
    TypeUse<T...> obj(*this, subexprs...);
    auto pos = std::find_if(_typeuseSt._ptrs.begin(), _typeuseSt._ptrs.end(), [&obj](const auto &p) { 
        return obj.eq(*p);
    });
    if (pos != _typeuseSt._ptrs.end()) {
        return *pos;
    }
    
    
    TypeUse* newPtr = new TypeUse<T...>(obj);
    _typeuseSt._ptrs.push_back(newPtr);
    return newPtr;
}

template <class ...T>
CExpr* Parser::structdecl(T ...subexprs) {
    StructDecl<T...> obj(*this, subexprs...);
    auto pos = std::find_if(_structdeclSt._ptrs.begin(), _structdeclSt._ptrs.end(), [&obj](const auto &p) { 
        return obj.eq(*p);
    });
    if (pos != _structdeclSt._ptrs.end()) {
        return *pos;
    }
    StructDecl* newPtr = new StructDecl<T...>(obj);
    _structdeclSt._ptrs.push_back(newPtr);
    return newPtr;
}

bool Parser::__recurs_check_cycles(CExpr* expr, CExpr* const par, std::unordered_set<CExpr*> &set,std::vector<std::string_view> &path) {
    if (expr == nullptr) {
        assert(false && "Faced nullptr!");
        return false;
    }
    if (set.contains(expr)) {
        path.push_back(_namedStorage.try_find_name(expr));
        return true;
    }
    const Seq *seq = dynamic_cast<const Seq*>(expr);
    if (seq) {
        set.insert(expr);

        if (seq->size() == 0) {
            assert(false && "Faced seq of size = 0");
        }
        
        CExpr *child = seq->at(0);
        bool isCycleInChild = __recurs_check_cycles(child, expr, set, path);
        if (isCycleInChild) {
            path.push_back(_namedStorage.try_find_name(expr));
        }
    
        set.erase(expr);
    
        return isCycleInChild;
    }

    const Or *or_ = dynamic_cast<const Or*>(expr);
    if (or_) {
        set.insert(expr);

        if (or_->size() == 0) {
            assert(false && "Faced seq of size = 0");
        }

        for (size_t i = 0; i < or_->size(); ++i) {
            CExpr *child = or_->at(i);
            bool isCycleInChild = __recurs_check_cycles(child, expr, set, path); 
            if (isCycleInChild) {
                path.push_back(_namedStorage.try_find_name(expr));
                return true;
            }
        }

        set.erase(expr);
        return false;
    }

    const Term* term = dynamic_cast<const Term*>(expr);
    if (term) {
        return false;
    }

    const RefExpr* ref = dynamic_cast<const RefExpr*>(expr);
    if (ref) {
        set.insert(expr);

        bool isCycleInChild = __recurs_check_cycles(ref->get_real_expr(), par, set, path);
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
    bool isCycleInChild = __recurs_check_cycles(e, nullptr, set, path);
    if (isCycleInChild) {
        for (size_t i = 0; i < path.size(); ++i) {
            std::cout<<"["<<i<<"]"<<":  "<<path[i]<<std::endl;
        }
    }
    return isCycleInChild;
}


#endif