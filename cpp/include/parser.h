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
    NamedStorage _namedStorage;

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