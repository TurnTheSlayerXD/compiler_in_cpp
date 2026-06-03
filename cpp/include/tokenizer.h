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
    X(".", DOT) \
    X(",", COMMA)

    
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
    COMMA,
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
        case TokenType::COMMA : return "[,]";
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


struct TokPos {
    size_t index;
};


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
    Cursor _startCur;

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

    Tokenizer(const Tokenizer &o) = delete;
    Tokenizer operator = (const Tokenizer &o) = delete;

    Tokenizer(Tokenizer &&o) = default;

    TokPos get_pos() {
        return TokPos{ .index = _tokIndex };
    }

    void reset_pos(TokPos p) {
        _tokIndex = p.index;
    }

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
        auto newToken = Token { 
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


    size_t manage_state() {
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
                add_token_and_reset(_lastSepType, _startCur, _lastSep);
                assert(_lastSep.size() > 0);
                return _lastSep.size();
            }
            else _state = State::INVALID;
            break;

        case State::SINGLE_QUOTE:
            if (reached_end()) _state = State::INVALID;
            else if (cur_char() == '\\') return 2;
            else if (cur_char() == '\'') {
                add_token_and_reset(TokenType::CHAR, _startCur, slice_self(_startCur.pos, _cur.pos));
            }
            break;

        case State::DOUBLE_QUOTE:
            if (reached_end()) _state = State::INVALID;
            else if (cur_char() == '\\') return 2;
            else if (cur_char() == '\"') {
                add_token_and_reset(TokenType::STRING, _startCur, slice_self(_startCur.pos, _cur.pos));
            }
            break;

        case State::WORD:
            if (reached_end() || lookup_for_sep() || isspace(cur_char())) { 
                add_token_and_reset(TokenType::WORD, _startCur, slice_self(_startCur.pos, _cur.pos)); 
                return 0;
            }
            else if (isalnum(cur_char())) {}
            else _state = State::INVALID;
            break;

        case State::NUM:
            if (reached_end())  {
                add_token_and_reset(TokenType::NUM_INT, _startCur, slice_self(_startCur.pos, _cur.pos));
                return 0;
            }
            else if (isdigit(cur_char())) {}
            else if (cur_char() == '.') _state = State::FLOAT; 
            else if (lookup_for_sep() || isspace(cur_char())) {
                add_token_and_reset(TokenType::NUM_INT, _startCur, slice_self(_startCur.pos, _cur.pos));
                return 0;
            }
            else _state = State::INVALID;
            break;

        case State::FLOAT:
            if (reached_end() || lookup_for_sep() || isspace(cur_char())) {
                add_token_and_reset(TokenType::NUM_FLOAT, _startCur, slice_self(_startCur.pos, _cur.pos));
                return 0;
            }
            else if (isdigit(cur_char())) {}
            else _state = State::INVALID;
            break;

        case State::DOT: 
            if (reached_end()) {
                add_token_and_reset(TokenType::DOT, _startCur, slice_self(_startCur.pos, _cur.pos));
                return 0;
            }
            else if (isdigit(cur_char())) _state = State::FLOAT;
            else {
                add_token_and_reset(TokenType::DOT, _startCur, slice_self(_startCur.pos, _cur.pos));
                return 0;
            }
            break;
        case State::PREPR:
            if (reached_end())  {}
            else if (cur_char() == '\n') {
                _state = State::START;
                trim_left();
                _startCur = _cur;
            }
            break;
        case State::INVALID: break;
        }

        return 1;
    }


    Token next_token() {
        if (_tokIndex < _tokens.size()) {
            _tokIndex += 1;
            return _tokens[_tokIndex - 1];
        }
    
        trim_left();
        _startCur = _cur;

        for (;;) {
            size_t prevTokensCount = _tokens.size();

            size_t timesToIter = manage_state();

            if (_state == State::INVALID) {
                _errBit = true;
                _errMsg = "Invalid Tokenizer state";
                return {};
            }

            if (reached_end() && _state != State::START) {
                _errBit = true;
                _errMsg = "next_token: Reached end of source while in MIDDLE state";
                return {};
            }
            
            for (size_t i = 0; i < timesToIter && !reached_end(); ++i) {
                iter();
            }
            
            if (_tokens.size() > prevTokensCount) {
                if (_state != State::START) {
                    assert(false && "Unreachable");
                }
                return _tokens.back();
            }
        }
    }

    
};

#endif
