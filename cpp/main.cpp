#include <iostream>
#include <tokenizer.h>
#include <array>


struct NamedExpr;
class Expr;
class Term;

struct NamedExpr {
    std::string_view name;
    Expr *expr;
};

class Expr {
public:
    static std::vector<NamedExpr> _namedExprs;

    virtual bool matches(Tokenizer &t) = 0;
    virtual ~Expr(){}

    void setName(std::string_view name) {
        auto pos = std::find_if(_namedExprs.begin(), _namedExprs.end(), [&name](const auto &arg){ return arg.name == name; });

        assert(pos == _namedExprs.end());
        _namedExprs.push_back( NamedExpr {.name = name, .expr = this});
    }
};

class Term: public Expr {
public:
    TokenType _tokType;
    Term(TokenType tokType): _tokType{tokType} {
    }

    bool matches(Tokenizer &t) override {
        if (t.eof()) {
            return false;
        }
        return _tokType == t.next_token().type;
    }
};

template <class ...T>
class Seq: public Expr {
public:

    std::array<Expr*, sizeof...(T)> _subexprs;

    size_t _index; 

    Seq(T ...seq): _index{0} {
        append(seq...);
    }

template <class ...U>
    void append(const char *name, U&& ...rest) {
        auto pos = std::find(_namedExprs.begin(), _namedExprs.end(), [&name](const auto &arg){ return arg.name == name; });
        assert(pos != _namedExprs.end());
        _subexprs[_index++] = *pos;
        append(rest...);
    }

template <class ...U>
    void append(const Expr &expr, U&& ...rest) {
        _subexprs[_index++] = &expr;
        append(rest...);
    }

template <class ...U>
    void append(TokenType tokType, U&& ...rest) {
        _subexprs[_index++] = &Term(tokType);
        append(rest...);
    }


    bool matches(Tokenizer &t) override {
        return false;
    }
};

int main() {

    auto tokenizer = Tokenizer("a + b");

    while (!tokenizer.eof()) {
        Token tok = tokenizer.next_token();
        if (tokenizer._errBit) {
            std::println("{}", tokenizer._errMsg);
            return 69;
        }
        std::println("{}", tok);
    }


    using enum TokenType;

    Seq(WORD, PLUS, WORD);

}