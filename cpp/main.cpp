#include <iostream>
#include <string>
#include <optional>
#include <vector>

#define MACRO_SEP_TOKENS \
    X(PLUS, "+")\
    X(MINUS, "-")\
    X(MUL, "*")\
    X(DIV, "/")\
    X(L_BR, "(")\
    X(R_BR, ")")\
    X(S_QUOTE, "'")\
    X(D_QUOTE, "\"")

enum class TokenType {
    PLUS,
    MINUS,
    MUL,
    DIV,
    L_BR,
    R_BR,
    WORD,
    S_QUOTE,
    D_QUOTE
};

struct Token {
    std::string_view text;
    TokenType type;
};

struct Cursor {
    size_t pos;
    size_t row;
    size_t col;
};

std::string_view slice(std::string_view s, size_t l, size_t r) {
    if (r > l) {
        throw std::exception("r > l");
    }
    return s.substr(l, r - l);
}

bool starts_with(std::string_view text, std::string_view pat) {
    if (pat.size() > text.size()) {
        return false;
    }
    for (size_t i = 0; i < pat.size(); ++i) {
        if (text[i] != pat[i]) {
            return false;
        }
    }
    return true;
}

class Tokenizer {

public:
    std::string_view _srcText;
    std::vector<Token> _tokens;

    size_t _tokIndex;
    Cursor _cur;

    TokenType _lastSep;
    
    Tokenizer(std::string_view text): 
        _srcText{text},  
        _tokIndex{0},
        _cur{0, 0, 0} {}

    std::string_view cur_text() {
        return slice(_srcText, _cur.pos, _srcText.size());
    }

    bool iter() {
        if (_cur.pos >= _srcText.size()) {
            return false;
        }
        _cur.pos += 1;
        if (_srcText[_cur.pos] == '\n') {
            _cur.row += 1;
            _cur.col = 0;
        }
        else {
            _cur.col += 1;
        }
    }

    bool eof() {
        return _cur.pos >= _srcText.size();
    }

    bool reached_end() {
        return _cur.pos >= _srcText.size();
    }


    bool trim_left() {
        while (isspace(src_text[cur.pos]) && iter()) {} 
    }

    Token next_token() {
        if (_tokIndex < _tokens.size()) {
            _tokIndex += 1;
            return _tokens[_tokIndex - 1];
        }
    
        for (;;) {
            if (reached_end()) {
                return {};
            }

            trim_left();

            if (starts_with(cur_text(), "#")) {
                while (!starts_with(cur_text(), "\n") && iter()) {
                }
            }

            if (lookup_for_sep()) {
                switch(_lastSep) {
                case TokenType::S_QUOTE: case TokenType::D_QUOTE:

                break;
                
                default:
                    
                }

            }
        }
    }

    bool lookup_for_sep() {
        #define X(type, str) \
            if (starts_with(cur_text(), (str))) {\
                 _lastSep = (TokenType::type); return true;\
            }
            MACRO_SEP_TOKENS
        #undef X
    }

};


int main() {

    auto src = Tokenizer("a + b");
    
    auto x = TokenType::PLUS;

    Token t;
    if (src.eof()) {

    }
}