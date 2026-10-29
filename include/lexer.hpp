#ifndef TOKENIZER_HPP

#define TOKENIZER_HPP

#include <iostream>
#include <cctype>
#include <optional>
#include <vector>
#include <algorithm>
#include <cassert>

#include "tokens_def.hpp"
#include "help.hpp"

struct Cursor {
    size_t pos;
    size_t row;
    size_t col;
    size_t tok_index;
};

std::string to_string(const Cursor& c) {
    return std::format("Row: {}, Col: {}", c.row, c.col);
}

struct Token {
    TokenType        type;
    Cursor           cur;
    std::string_view text;
    size_t           tok_index;
};

std::string to_string(const Token &tok) {
    return std::format("{} at {}:{} [{}]", to_string(tok.type), tok.cur.row, tok.cur.col, tok.text);
}

std::ostream& operator<<(std::ostream& str, const Token& t) {
    str << "Token " << to_string(t.type) << "," << " " << "`" << t.text << "`";
    return str;
}

class Lexer {
    static constexpr char CHAR_EOF = '\0';
    enum class State {
        START,
        EXP,
        DIGIT,
    };
public:
    std::string_view m_text;
    std::vector<Token> m_tokens = {};
    Cursor m_cur = {0, 0, 0};
    size_t m_tok_index = 0;
    bool m_err_bit = true;
    std::vector<std::string> m_errs = {};

    Lexer(std::string_view text):
        m_text{text}
    {
    }

    Lexer(Lexer &&o) noexcept              = default;
    Lexer& operator = (Lexer &&o) noexcept = default;

    Lexer(const Lexer &o)                  = delete;
    Lexer& operator = (const Lexer &o)     = delete;

    Cursor get_pos() {
        return m_cur;
    }

    void make_token(TokenType type, Cursor cur, std::string_view text) {
        auto t = Token {
            .type     = type,
            .cur      = cur,
            .text     = text,
            .tok_index= m_tokens.size(),
        };
        m_tokens.push_back(t);
    }

    void read_char() {
        if (m_cur.pos >= m_text.size()) return;
        if (m_text[m_cur.pos] == '\n') { m_cur.row += 1; m_cur.col = 0; }
        else { m_cur.col += 1; }
        ++m_cur.pos;
        return;
    }

    char cur_char() {
        if (m_cur.pos>=m_text.size()) return CHAR_EOF;
        return m_text[m_cur.pos];
    }

    bool lookup_str(Cursor from_cur, std::string_view str) {
        if (str.size() > m_text.size() - from_cur.pos) return false;
        return slice(m_text, from_cur.pos, from_cur.pos + str.size()) == str;
    }

    bool is_eof() {
        return cur_char() == CHAR_EOF;
    }

    std::string get_err() {
        std::string err_tot;
        for (auto &err: m_errs) {
            err_tot += err + "\n";
        }
        return err_tot;
    }

    void set_unmatched_sym_err(Cursor cur, TokenType ttype) {
        m_err_bit = true;
        m_errs.push_back(std::format("Unmatched sym [{}] at {}", to_string(ttype), to_string(cur)));
    }

    void set_unknown_token_err(Cursor cur, std::string_view str) {
        m_err_bit = true;
        m_errs.push_back(std::format("Unknown sym [{}] at {}", str, to_string(cur)));
    }

    bool _produce_token() {
        // Trimming text
        while (std::isspace(cur_char())) {
            read_char();
        }

        if (is_eof()) {
            return false;
        }

        Cursor start_cur = m_cur;
        TokenType ttype;

        std::string_view longest_sep = "";

        #define X(SEP_STR, SEP_TP) if (lookup_str(start_cur, SEP_STR) && std::strlen(SEP_STR) > longest_sep.size()) { longest_sep = SEP_STR; ttype = TokenType::SEP_TP; }
        MACRO_SEPARATORS
        #undef X
        if (!longest_sep.empty()) {
            for (size_t i = 0; i < longest_sep.size(); ++i) read_char();
            make_token(ttype, start_cur, slice(m_text, start_cur.pos, m_cur.pos));
            return true;
        }

        std::string_view longest_kwd;
        #define X(KWD_STR, KWD_TP) if (lookup_str(start_cur, KWD_STR) && std::strlen(KWD_STR) > longest_kwd.size()) { longest_kwd = KWD_STR; ttype = TokenType::KWD_TP; }
        MACRO_KWDS
        #undef X

        if (!longest_kwd.empty()) {
            for (size_t i = 0; i < longest_kwd.size(); ++i) read_char();
            make_token(ttype, start_cur, slice(m_text, start_cur.pos, m_cur.pos));
            return true;
        }

        if (cur_char() == '\'') {
            bool matched = false;
            do {
                read_char();
                if      (cur_char() == '\\')  {                  read_char();}
                else if (cur_char() == '\'')  { matched = true; read_char(); }
            }
            while (!is_eof() && !matched);

            if (!matched) {
                set_unmatched_sym_err(start_cur, TokenType::LIT_CHAR);
                return false;
            }
            make_token(TokenType::LIT_CHAR, start_cur, slice(m_text, start_cur.pos+1, m_cur.pos-1));
            return true;
        }
        if (cur_char() == '"') {
            bool matched = false;
            do {
                read_char();
                if      (cur_char() == '\\') {                 read_char(); }
                else if (cur_char() == '"')  { matched = true; read_char(); }
            }
            while (!is_eof() && !matched);

            if (!matched) {
                set_unmatched_sym_err(start_cur, TokenType::LIT_STRING);
                return false;
            }
            make_token(TokenType::LIT_STRING, start_cur, slice(m_text, start_cur.pos+1, m_cur.pos-1));
            return true;
        }

        bool is_num = std::isdigit(cur_char());
        while (is_num && !is_eof() && std::isdigit(cur_char())) {
            read_char();
        }
        if (is_num) {
            make_token(TokenType::LIT_INT, start_cur, slice(m_text, start_cur.pos, m_cur.pos));
            return true;
        }


        bool is_word = std::isalpha(cur_char());
        while (is_word && !is_eof() && std::isalnum(cur_char()))
            read_char();
        if (is_word) {
            make_token(TokenType::WORD, start_cur, slice(m_text, start_cur.pos, m_cur.pos));
            return true;
        }

        set_unknown_token_err(start_cur, slice(m_text, start_cur.pos, m_cur.pos));
        return false;
    }

    bool consume_token(Token *ret_token) {
        if (m_tok_index == m_tokens.size()) {
            bool did_produce = _produce_token();
            if (!did_produce) return false;
        }
        *ret_token = m_tokens[m_tok_index++];
        return true;
    }

    lookup_token(TokenType ttype) {
        if ()
    }

    bool consume_on_match(TokenType ...toks) {
    }
};

#endif
