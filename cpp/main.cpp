#include <iostream>
#include <tokenizer.h>
#include <array>

template <class T>
struct Destruct {
    T _cbk;
    Destruct(T cbk): _cbk{cbk} { }
    ~Destruct() {
        _cbk();
    }
};



class Expr;
class Term;


enum class NodeType {
    PLUS,
    LEAF,
};

struct Node {
    std::vector<Node*> children;
    NodeType _type;
    Node(NodeType type): _type{type} {
    }

    
    ~Node() {
        for (auto child: children) {
            delete child;
        }
    }
};

struct NamedStorage {    
    struct NamedExpr {
        std::string_view name;
        Expr *expr;
    };

    std::vector<NamedExpr> _items;
    void setName(Expr* expr, std::string_view name) {
        auto pos = std::find_if(_items.begin(), _items.end(), [&name](const auto &arg){ return arg.name == name; });
        assert(pos == _items.end());
        _items.push_back( NamedExpr {.name = name, .expr = expr});
    }
    Expr* find(std::string_view name) {
        auto pos = std::find_if(_items.begin(), _items.end(), [&name](const auto &arg){ return arg.name == name; });
        assert(pos != _items.end());
        return pos->expr;
    }
};
static NamedStorage ST;

class Expr {
public:
    NodeType _nodeType;

    Expr(NodeType nodeType): _nodeType{nodeType} {}

    virtual Node* eval(Tokenizer &t) = 0;
    virtual ~Expr(){}

    void setName(std::string_view name) {
        ST.setName(this, name);
    }
};

class Term: public Expr {
public:
    TokenType _tokType;
    Term(TokenType tokType): Expr(NodeType::LEAF), _tokType{tokType} {}

    Node* eval(Tokenizer &t) override {
        Node *v = new Node(_nodeType);
        bool ok = true;
        auto initPos = t.get_pos();
        Destruct onDestruct([&ok, &v, &t, &initPos]{ if (!ok) { t.reset_pos(initPos); delete v; }});        

        if (t.eof() || t.next_token().type != _tokType) {
            ok = false;
            return nullptr;
        }

        return new Node(_nodeType); 
    }
};

class Seq: public Expr {
public:

    Seq(NodeType nodeType): Expr(nodeType){}

    virtual Expr* at(size_t index) = 0;
    virtual size_t size() = 0;
};

template <class ...T>
class TemplateSeq: public Seq {
public:
    std::array<Expr*, sizeof...(T)> _subexprs;
    size_t _index; 

    TemplateSeq(NodeType nodeType, T ...subexprs): Seq(nodeType), _index{0} {
        append(subexprs...);
    }

    Node* eval(Tokenizer &t) override {
        Node *v = new Node(_nodeType);
        bool ok = true;
        auto initPos = t.get_pos();
        Destruct onDestruct([&ok, &v, &t, &initPos]{ if (!ok) { t.reset_pos(initPos); delete v; }});        

        for (auto subexpr: _subexprs) {
            Node *res = subexpr->eval(t);
            if (!res) {
                ok = false;
                break;
            }
            v->children.push_back(res);
        }

        if (!ok) {
            return nullptr;
        }

        return v;
    }

    Expr* at(size_t index) override {
        return _subexprs[index];
    }
    
    size_t size() override {
        return sizeof...(T);
    }

    template <class ...U>
    void append(const char *name, U ...rest) {
        auto expr = ST.find(name);
        _subexprs[_index++] = expr;
        append(rest...);
    }

    template <class ...U>
    void append(Expr *expr, U ...rest) {
        _subexprs[_index++] = expr;
        append(rest...);
    }

    template <class ...U>
    void append(TokenType tokType, U ...rest) {
        _subexprs[_index++] = term(tokType);
        append(rest...);
    }


    void append() {
    }
};

struct TermStorage {
    Term *ptr;
    size_t len;
    size_t cap;
};

void push_TermStorage(TermStorage* st, Term newTerm){
    if (st->len >= st->cap) {
        st->ptr = (Term*)realloc(st->ptr, (st->cap * 2 + 1) * sizeof(Term));
        st->cap = st->cap * 2 + 1;
    }
    st->ptr[st->len++] = newTerm;
}

template <class T>
struct PtrStorage {
    std::vector<T*> _ptrs;
    ~PtrStorage() {
        for (auto p: _ptrs) {
            delete p;
        }
    }
};

Expr* term(TokenType tokType) {
    static PtrStorage<Term> st;
    auto pos = std::find_if(st._ptrs.begin(), st._ptrs.end(), [&tokType](const auto &p) {
        return p->_tokType == tokType;
    });
    if (pos != st._ptrs.end()) {
        return *pos;
    }
    auto newTerm = new Term(tokType);
    st._ptrs.push_back(newTerm);
    return newTerm;
}

template <class ...T>
Expr* seq(NodeType nodeType, T ...subexprs) {
    static PtrStorage<Seq> st;
    TemplateSeq<T...> newSeq(nodeType, subexprs...);
    auto pos = std::find_if(st._ptrs.begin(), st._ptrs.end(), [&newSeq](const auto &p) { 
        if (p->size() != newSeq.size()) {
            return false;
        }
        for (size_t i = 0; i < newSeq.size(); ++i) {
            if (newSeq.at(i) != p->at(i)) {
                return false;
            }
        }
        return true;
    });
    if (pos != st._ptrs.end()) {
        return *pos;
    }
    Seq* newSeqPtr = new TemplateSeq<T...>(newSeq); 
    st._ptrs.push_back(newSeqPtr);
    return newSeqPtr;
}


int main() {
    Tokenizer tokenizer("a + b");
    
    using enum TokenType;
    auto rule1 = seq(NodeType::PLUS,/**/WORD, PLUS, WORD/**/);
    Node *root = rule1->eval(tokenizer);

    std::printf("Root ptr = %p", root);
    delete root;
}