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
    Op,
    Op_Un,
    Op_Brace,
    Leaf,
};
std::string_view to_string(NodeType tp) {
    using enum NodeType;
    switch (tp) {
        case Op: return "Op";
        case Op_Un: return "Unar Op";
        case Op_Brace: return "Brace Op";
        case Leaf: return "Leaf";
        default: assert(false && "Unexpected"); return "";
    }
}

template <>
struct std::formatter<NodeType> : std::formatter<std::string> {
  auto format(NodeType type, format_context& ctx) const {
    return formatter<string>::format(
      std::format("{}", to_string(type)), ctx);
  }
};


struct NodePrintOpts {
    size_t indentStep = 4;

} po;

struct Node {
    

    std::vector<Node*> children;
    NodeType _type;
    Token relatedToken;

    Node(NodeType type): _type{type} {
    }
    
    ~Node() {
        for (auto child: children) {
            delete child;
        }
    }

    std::string_view to_string() {
        return ::to_string(_type);
    }

    std::string get_str_repr() {
        std::string buf;
        _print(0, this, buf);
        return buf;
    }

    static void set_indent(size_t count, std::string&buf) {
        for (size_t i = 0; i < count; ++i) {
            buf.push_back(' ');
        }
    }

    static void _print(size_t indent, Node *v, std::string &buf) {
        set_indent(indent, buf);
        buf += v->to_string();
        if (v->children.size() > 0) {
            buf += ": {";
            buf += "\n\r";
            for (auto child: v->children) {
                _print(indent + po.indentStep, child, buf);
            }
            set_indent(indent, buf);
            buf += "},\n\r";
        }
        else {
            buf += ",\n\r"; 
        }
    }

};

struct NamedStorage {    
    struct ExprWithName {
        std::string_view name;
        Expr *expr;
    };

    std::vector<ExprWithName> _items;
    void setName(Expr* expr, std::string_view name) {
        auto pos = std::find_if(_items.begin(), _items.end(), [&name](const auto &arg){ return arg.name == name; });
        assert(pos == _items.end());
        _items.push_back( ExprWithName {.name = name, .expr = expr});
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

    virtual Node* eval(Tokenizer &t) const = 0;
    virtual ~Expr(){}

    void setName(std::string_view name) {
        ST.setName(this, name);
    }
};

Expr* named_ref(std::string_view exprName);

Expr* term(TokenType tokType);

template <class ...T>
Expr* seq(NodeType nodeType, T ...subexprs);

template <class ...T>
Expr* or_seq(NodeType nodeType, T ...subexprs);


class RefExpr: public Expr {
public:
    std::string_view _exprName;
    RefExpr(std::string_view exprName): _exprName{exprName} {    }
    Node* eval(Tokenizer &t) const {
        auto expr = ST.find(_exprName);
        return expr->eval(t);
    }

};

class Term: public Expr {
public:
    NodeType _nodeType;
    TokenType _tokType;

    Term(TokenType tokType): _nodeType{NodeType::Leaf}, _tokType{tokType} {}

    Node* eval(Tokenizer &t) const override {
        Node *v = new Node(_nodeType);
        bool ok = true;
        auto initPos = t.get_pos();
        Destruct onDestruct([&ok, &v, &t, &initPos]{ if (!ok) { t.reset_pos(initPos); delete v; } else std::cout << v->get_str_repr() << std::endl; });        

        if (t.eof() || t.next_token().type != _tokType) {
            ok = false;
            return nullptr;
        }

        return new Node(_nodeType); 
    }

};

class Seq: public Expr {
public:
    NodeType _nodeType;
    
    Seq(NodeType nodeType): _nodeType{nodeType}{}
    
    virtual Expr* at(size_t index) const = 0;
    
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
    std::array<Expr*, sizeof...(T)> _subexprs;
    
    size_t _index; 
    
    TemplateSeq(NodeType nodeType, T ...subexprs): Seq(nodeType), _index{0} {
        append(subexprs...);
    }
    
    Node* eval(Tokenizer &t) const override {
        Node *v = new Node(_nodeType);
        bool ok = true;
        auto initPos = t.get_pos();
        Destruct onDestruct([&ok, &v, &t, &initPos]{ if (!ok) { t.reset_pos(initPos); delete v; } else std::cout << v->get_str_repr() << std::endl; });        
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
    
    Expr* at(size_t index) const override {
        return _subexprs[index];
    }
    
    size_t size() const override {
        return sizeof...(T);
    }
    
    template <class ...U>
    void append(const char *name, U ...rest) {
        _subexprs[_index++] = named_ref(name);
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

class Or: public Expr {
public:

    virtual Expr* at(size_t index) const = 0;

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

    std::array<Expr*, sizeof...(T)> _subexprs;
    
    size_t _index; 
    
    TemplateOr(T ...subexprs): _index{0} {
        append(subexprs...);
    }
    
    Node* eval(Tokenizer &t) const override {
        bool ok = true;
        auto initPos = t.get_pos();
        Destruct onDestruct([&ok, &t, &initPos]{ if (!ok) { t.reset_pos(initPos); }  });        
        for (auto subexpr: _subexprs) {
            Node *res = subexpr->eval(t);
            if (res) {
                std::cout << res->get_str_repr() << std::endl;
                return res;
            }
            t.reset_pos(initPos);
        }
        ok = false;
        return nullptr;
    }

    Expr* at(size_t index) const override {
        return _subexprs[index];
    }

    size_t size() const override {
        return sizeof...(T);
    }
    
    template <class ...U>
    void append(const char *name, U ...rest) {
        _subexprs[_index++] = named_ref(name);
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

template <class T>
struct PtrStorage {
    std::vector<T*> _ptrs;
    ~PtrStorage() {
        for (auto p: _ptrs) {
            delete p;
        }
    }
};


Expr* named_ref(std::string_view exprName) {
    static PtrStorage<RefExpr> st;
    auto pos = std::find_if(st._ptrs.begin(), st._ptrs.end(), [&exprName](const auto &p) {
        return p->_exprName == exprName;
    });
    if (pos != st._ptrs.end()) {
        return *pos;
    }
    auto newTerm = new RefExpr(exprName);
    st._ptrs.push_back(newTerm);
    return newTerm;
}

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
    TemplateSeq<T...> obj(nodeType, subexprs...);
    auto pos = std::find_if(st._ptrs.begin(), st._ptrs.end(), [&obj](const auto &p) { 
        return obj.eq(*p);
    });
    if (pos != st._ptrs.end()) {
        return *pos;
    }
    Seq * newPtr = new TemplateSeq<T...>(obj);
    st._ptrs.push_back(newPtr);
    return newPtr;
}

template <class ...T>
Expr* or_(T ...subexprs) {
    static PtrStorage<Or> st;
    TemplateOr<T...> obj(subexprs...);
    auto pos = std::find_if(st._ptrs.begin(), st._ptrs.end(), [&obj](const auto &p) { 
        return obj.eq(*p);
    });
    if (pos != st._ptrs.end()) {
        return *pos;
    }
    Or * newPtr = new TemplateOr<T...>(obj);
    st._ptrs.push_back(newPtr);
    return newPtr;
}




int main() {
    Tokenizer tokenizer("((a) + (b * d)) * (d + 69)");
    // while (!tokenizer.eof()) {
    //     tokenizer.next_token();
    // }
    // for (auto t : tokenizer._tokens) {
    //     std::println("{}", t);
    // }

    using enum TokenType;

    auto opSign = or_(PLUS, MINUS, MUL, DIV);

    auto operand = or_(WORD, NUM_INT, NUM_FLOAT);

    auto ruleBinOp = seq(
        NodeType::Op,
    /**/or_("brace_op", operand), opSign, or_("bin_op", "brace_op", operand)/**/);

    ruleBinOp->setName("bin_op");

    auto ruleBrace = seq(
        NodeType::Op_Brace,
    /**/L_BR, or_("bin_op", "brace_op", operand),  R_BR/**/
    );
    ruleBrace->setName("brace_op");

    Node *root = ruleBinOp->eval(tokenizer);

    std::printf("Root ptr = %p\n", root == nullptr ? 0 : root);

    if (root) {
        auto res = root->get_str_repr();
        std::cout << res << std::endl;
    }

    delete root;
}


//(((a) + (b)) + (c * d))