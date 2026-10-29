
#include <iostream>
#include "include/lexer.hpp"

struct Ast_Node {

};

struct Tp_Node: Ast_Node {
};

struct Tp_Name_Node: Tp_Node {
    std::string_view tp_name;

};

struct Tp_Const_Qual_Node: Tp_Node {
    Tp_Node *const_to;
};

struct Tp_Ptr_Qual_Node: Tp_Node {
    Tp_Node *ptr_to;
};


template <class T>
struct ParseResult {
    T *node;
    bool success;
};

class Parser {
    std::vector<AstNode*> node_heap;

    Lexer &m_lexer;
    Parser(Lexer &lex): m_lexer{lexer} {}

    ~Parser() {
        for (auto &p: node_heap) {
            delete p;
            p = nullptr;
        }
    }

    template <class T>
    T* alloc_node(T def) {
        node_heap.push_back(new T(std::move(def)));
        return node_heap.back();
    }

    bool is_valid_type(std::string_view tpname) {

    }

    ParseResult<Tp_Node> parse_tp() {
        bool is_const = false;
        while (m_lexer.consume_on_match(TokenType::KWD_CONST)) {
             is_const = true;
        }

        Token t;
        if (!m_lexer.consume_token(&t) || t.type != TokenType::WORD || !t.is_valid_type(t.text)) {
            return ParseResult<Tp_Node>::unsuccessful();
        }
        Tp_Node *out_tp = alloc_node(Tp_Name_Node{.tp_name = t.text});
        if (is_const) {
            out_tp = alloc_node(Tp_Const_Qual_Node{.const_to = out_tp});
        }

        while (m_lexer.consume_on_match(TokenType::ASTERISC)) {
            out_tp = alloc_node(Tp_Ptr_Qual_Node{.ptr_to = out_tp});
        }

        return ParseResult<Tp_Node>::successful(out_tp);
    }

    parse_fun_decl

    ParseResult parse() {

        // Deciding whether it is function or variable
        auto res = exec_parse(*this, parse_tp);
        if (!res.success) {
            return res;
        }

        Token tok;
        if (!m_lexer.consume_token(&tok) || t.type != TokenType::WORD || !) {

        }

    }
};

ParseResult<Tp_Node> exec_parse(Parser &parser, T fun) {
    auto pos = parser.m_lexer.get_pos();
    auto res = fun(parser);
    if (!res.success) {
        m_lexer.reset_pos(pos);
    }
    return res;
}


int main() {

    const char *prog = "int main(){\n"
        "printf(\"Hello, world\")}";

    Lexer lex{prog};

    Token t;
    while (lex.consume_token(&t)) {
        std::println("{}\n", to_string(t));
    }

}
