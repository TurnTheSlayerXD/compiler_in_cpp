#ifndef TOKENIZER_H

#define TOKENIZER_H

#include <string>
#include <optional>
#include <vector>
#include <cassert>
#include <print>

#define MACRO_SEPARATORS \
    X("+", PLUS) \
    X("-", MINUS) \
    X("'", CHAR) \
    X("\"", STRING) \
    X("*", MUL) \
    X("/", DIV) \
    X("(", L_BR) \
    X(")", R_BR) \
    X(";", SEMICOLON) \
    X(".", DOT)
    
enum class TokenType {
    PLUS,
    MINUS,
    MUL,
    DIV,
    L_BR,
    R_BR,
    WORD,
    CHAR,
    STRING,
    NUM_INT,
    NUM_FLOAT,
    SEMICOLON,
    DOT,
};

std::string_view to_string(TokenType t) {
    switch(t) {
        case TokenType::PLUS : return "[+]";
        case TokenType::MINUS : return "[-]";
        case TokenType::MUL : return "[*]";
        case TokenType::DIV : return "[/]";
        case TokenType::L_BR : return "[(]";
        case TokenType::R_BR : return "[)]";
        case TokenType::WORD : return "[word]";
        case TokenType::NUM_INT : return "[num INT]";
        case TokenType::NUM_FLOAT : return "[num FLOAT]";
        case TokenType::CHAR : return "[char]";
        case TokenType::STRING : return "[string]";
        default: assert(false && "UNREACHABLE"); return "";
    }
}

template <>
struct std::formatter<TokenType> : std::formatter<std::string> {
  auto format(TokenType type, format_context& ctx) const {
    return formatter<string>::format(
      std::format("{}", to_string(type)), ctx);
  }
};


struct Cursor {
    size_t pos;
    size_t row;
    size_t col;
};

struct Token {
    TokenType        type;
    Cursor           cur;
    std::string_view text;
    size_t           tokIndex;

};

template <>
struct std::formatter<Token>: std::formatter<std::string> {
  auto format(Token t, format_context& ctx) const {
    return formatter<string>::format(
      std::format("Token {}, str=`{}`", t.type, t.text), ctx);
  }
};

std::string_view slice(std::string_view s, size_t l, size_t r) {
    assert(l <= r && "slice: l > r");
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

    enum class State {
        START,
        NUM,
        FLOAT,
        WORD,
        PREPR,
        DOT,
        SINGLE_QUOTE,
        DOUBLE_QUOTE,
        INVALID,
    };

public:
    std::string_view _srcText;
    std::vector<Token> _tokens;

    size_t _tokIndex;
    Cursor _cur;

    std::string_view _lastSep;
    TokenType _lastSepType;

    bool _errBit;
    std::string_view _errMsg;

    State _state;
    
    Tokenizer(std::string_view text): 
        _srcText{text},  
        _tokIndex{0},
        _cur{0, 0, 0},
        _errBit{false},
        _state{State::START} {}

    std::string_view cur_text() {
        return slice(_srcText, _cur.pos, _srcText.size());
    }

    char cur_char() {
        assert(_cur.pos < _srcText.size() && "cur_char: out of bounds");
        return _srcText[_cur.pos];
    }

    bool is_cur_matches(std::string_view pat) {
        assert(_cur.pos < _srcText.size() && "is_cur_matches: out of bounds");
        assert(pat.size() > 0 && "is_cur_matches: patterns length is 0");
        if (pat.size() == 1) {
            return _srcText[_cur.pos] == pat[0];
        }
        return starts_with(cur_text(), pat);
    }

    bool iter() {
        assert(_cur.pos < _srcText.size() && "iter(): out of iterations");
        if (_srcText[_cur.pos] == '\n') {
            _cur.row += 1;
            _cur.col = 0;
        }
        else {
            _cur.col += 1;
        }
        _cur.pos += 1;
        if (_cur.pos == _srcText.size()) {
            return false;
        }
        return true;
    }

    bool eof() {
        if (_tokIndex < _tokens.size()) {
            return false;
        }
        trim_left();
        return _cur.pos >= _srcText.size();
    }

    bool reached_end() {
        assert(_cur.pos <= _srcText.size() && "reached_end: out of bounds");
        return _cur.pos == _srcText.size();
    }

    void trim_left() {
        if (reached_end()) {
            return;
        }
        while (isspace(_srcText[_cur.pos]) && iter()) {} 
    }


    Token add_token(TokenType type, Cursor curOfToken, std::string_view text) {
        auto newToken = (Token){ 
            .type = type ,
            .cur = curOfToken ,
            .text = text ,
            .tokIndex = _tokIndex,
        };
        assert(_tokIndex == _tokens.size());
        _tokens.push_back(newToken);
        _tokIndex += 1;
        return newToken;
    }

    Token add_token_and_reset(TokenType type, Cursor cur, std::string_view text) {
        _state = State::START;
        return add_token(type, cur, text);
    }

    bool lookup_for_sep() {
        #define X(str, type) \
            if (is_cur_matches(str)) {\
                _lastSep = str; \
                _lastSepType = (TokenType::type); \
                return true; \
            }
            MACRO_SEPARATORS
        #undef X

        return false;
    }

    std::string_view slice_self(size_t l, size_t r) {
        assert(l < r && r <= _srcText.size());
        return slice(_srcText, l, r);
    }

    Token next_token() {
        if (_tokIndex < _tokens.size()) {
            _tokIndex += 1;
            return _tokens[_tokIndex - 1];
        }
    
        trim_left();
        Cursor startCur = _cur;

        for (;;) {
            switch (_state) {
            case State::START:
                if (reached_end()) {}
                else if (cur_char() == '.') _state = State::DOT;
                else if (cur_char() == '\'') _state = State::SINGLE_QUOTE;
                else if (cur_char() == '"') _state = State::DOUBLE_QUOTE;
                else if (isalpha(cur_char())) _state = State::WORD;
                else if (isdigit(cur_char())) _state = State::NUM;
                else if (cur_char() == '#') _state = State::PREPR;
                else if (lookup_for_sep()) {
                    auto newToken = add_token_and_reset(_lastSepType, startCur, _lastSep);
                    assert(_lastSep.size() > 0);
                    for (size_t i = 0; i < _lastSep.size(); ++i) {
                        iter();
                    }
                    return newToken;
                }
                else _state = State::INVALID;
                break;

            case State::SINGLE_QUOTE:
                if (reached_end()) _state = State::INVALID;
                else if (cur_char() == '\\')
                    iter();
                else if(cur_char() == '\'') {
                    auto endCur = _cur;
                    iter();
                    return add_token_and_reset(TokenType::CHAR, startCur, slice_self(startCur.pos, endCur.pos));
                }
                break;

            case State::DOUBLE_QUOTE:
                if (reached_end()) _state = State::INVALID;
                if (cur_char() == '\\')
                    iter();
                else if(cur_char() == '\"') {
                    auto endCur = _cur;
                    iter();
                    return add_token_and_reset(TokenType::STRING, startCur, slice_self(startCur.pos, endCur.pos));
                }
                break;

            case State::WORD:
                if (reached_end() || lookup_for_sep() || isspace(cur_char()))
                    return add_token_and_reset(TokenType::WORD, startCur, slice_self(startCur.pos, _cur.pos));
                
                if (isalnum(cur_char())) {} 
                else _state = State::INVALID;
                break;

            case State::NUM:
                if (reached_end())
                    return add_token_and_reset(TokenType::NUM_INT, startCur, slice_self(startCur.pos, _cur.pos));
                else if (isdigit(cur_char())) {}
                else if (cur_char() == '.') _state = State::FLOAT;
                else if (lookup_for_sep() || isspace(cur_char()))
                    return add_token_and_reset(TokenType::NUM_INT, startCur, slice_self(startCur.pos, _cur.pos));
                else _state = State::INVALID;
                break;

            case State::FLOAT:
                if (reached_end())
                    return add_token_and_reset(TokenType::NUM_FLOAT, startCur, slice_self(startCur.pos, _cur.pos));
                else if (isdigit(cur_char())) {}
                else if (lookup_for_sep() || isspace(cur_char()))
                    return add_token_and_reset(TokenType::NUM_FLOAT, startCur, slice_self(startCur.pos, _cur.pos));
                else _state = State::INVALID;
                break;

            case State::DOT: 
                if (reached_end()) {
                    iter();
                    return add_token_and_reset(TokenType::DOT, startCur, slice_self(startCur.pos, _cur.pos));
                }
                else if (isdigit(cur_char())) _state = State::FLOAT;
                else {
                    iter();
                    return add_token_and_reset(TokenType::DOT, startCur, slice_self(startCur.pos, _cur.pos));
                }
                break;
            case State::PREPR:
                if (reached_end()) 
                    break;
                if (cur_char() == '\n') {
                    _state = State::START;
                    trim_left();
                    startCur = _cur;
                }
                break;
            case State::INVALID: break;
            }


            if (_state == State::INVALID) {
                _errBit = true;
                _errMsg = "Invalid Tokenizer state";
                return {};
            }
            else if (reached_end() && _state != State::START) {
                _errBit = true;
                _errMsg = "next_token: Reached end of source while in MIDDLE state";
                return {};
            }

            iter();
        }
    }
};

#endif TOKENIZER_H