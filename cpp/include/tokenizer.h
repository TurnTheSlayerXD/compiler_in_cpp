#ifndef TOKENIZER_H

#define TOKENIZER_H

#include <string>
#include <optional>
#include <vector>
#include <algorithm>

#include <cassert>

#define MACRO_SEPARATORS \
    X("=",  ASSIGN) \
    X("+",  PLUS) \
    X("-",  MINUS) \
    X("*",  MUL) \
    X("/",  DIV) \
    X("&",  ADDR)\
    X("++", INCR) \
    X("--", DECR) \
    X(">",  GR)\
    X("<",  LE)\
    X(">=", GR_E)\
    X("<=", LE_E)\
    X("==", EQ)\
    X("&&", AND)\
    X("||", OR)\
    X("'",  CHAR) \
    X("\"", STRING) \
    X(";",  SEMICOLON) \
    X(".",  DOT) \
    X(",",  COMMA) \
    X("(",  L_BR) \
    X(")",  R_BR) \
    X("[",  L_SUBSCR) \
    X("]",  R_SUBSCR)\
    X("{",  L_CURL)\
    X("}",  R_CURL)

#define MACRO_KWDS\
    X("const",    KWD_CONST)\
    X("return",   KWD_RET)\
    X("struct",   KWD_STRUCT)\
    X("for",      KWD_FOR)\
    X("while",    KWD_WHILE)\
    X("break",    KWD_BREAK)\
    X("continue", KWD_CONTINUE)\
    X("do",       KWD_DO)\
    X("if",       KWD_IF)\
    X("else",     KWD_ELSE)\
    X("switch",   KWD_SWITCH)\
    X("case",     KWD_CASE)
    
enum class TokenType {
    ASSIGN,
    PLUS,
    MINUS,
    MUL,
    DIV,
    ADDR,
    INCR,
    DECR,
    GR,
    LE,
    GR_E,
    LE_E,
    EQ,
    AND,
    OR,
    CHAR,
    STRING,
    WORD,
    NUM_INT,
    NUM_FLOAT,
    SEMICOLON,
    DOT,
    COMMA,
    L_BR,
    R_BR,
    L_SUBSCR,
    R_SUBSCR,
    L_CURL,
    R_CURL,
    KWD_CONST,
    KWD_RET,
    KWD_STRUCT,
    KWD_FOR,
    KWD_WHILE,
    KWD_BREAK,
    KWD_CONTINUE,
    KWD_DO,
    KWD_IF,
    KWD_ELSE,
    KWD_SWITCH,
    KWD_CASE,
};

constexpr auto MACRO_SEP_COUNT = std::size({
    #define X(str, tok) std::make_tuple((str), (TokenType::tok)),
        MACRO_SEPARATORS
    #undef X
});


std::string_view to_string(TokenType t) {
    using enum TokenType;
    switch(t) {
        case PLUS : return "`+`";
        case MINUS : return "`-`";
        case MUL : return "`*`";
        case DIV : return "`/`";
        case L_BR : return "`(`";
        case R_BR : return "`)`";
        case WORD : return "`word`";
        case NUM_INT : return "`num INT`";
        case NUM_FLOAT : return "`num FLOAT`";
        case CHAR : return "`char`";
        case STRING : return "`string`";
        case COMMA : return "`,`";
        case L_SUBSCR : return "`[`";
        case R_SUBSCR : return "`]`";
        case ASSIGN : return "`=`";
        case INCR : return "`++`";
        case DECR : return "`--`";
        case GR : return "`>`";
        case LE : return "`<`";
        case GR_E : return "`>=`";
        case LE_E : return "`<=`";
        case EQ : return "`==`";
        case AND : return "`&&`";
        case OR : return "`||`";
        case ADDR : return "`&`";
        case KWD_CONST : return "`const`";
        case KWD_RET : return "`return`";
        case KWD_STRUCT : return "`struct`";
        case KWD_FOR : return "`for`";
        case KWD_WHILE : return "`while`";
        case KWD_BREAK : return "`break`";
        case KWD_CONTINUE : return "`continue`";
        case KWD_DO : return "`do`";
        case KWD_IF : return "`if`";
        case KWD_ELSE : return "`else`";
        case KWD_SWITCH : return "`switch`";
        case KWD_CASE : return "`case`";
        case SEMICOLON : return "`;`";
        case L_CURL: return "`{`";
        case R_CURL: return "`}`";

        default: { fprintf(stderr, "\nTo_string not implemented for: [%d]\n", static_cast<int>(t)); assert(false && "UNREACHABLE"); return "";}
    }
}

// template <>
// struct std::formatter<TokenType> : std::formatter<std::string> {
//   auto format(TokenType type, format_context& ctx) const {
//     return formatter<string>::format(
//       std::format("{}", to_string(type)), ctx);
//   }
// };

std::ostream& operator<<(std::ostream& o, TokenType t) {
    o << to_string(t);
    return o;
}


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

// template <>
// struct std::formatter<Token>: std::formatter<std::string> {
//   auto format(Token t, format_context& ctx) const {
//     return formatter<string>::format(
//       std::format("Token {}, str=`{}`", t.type, t.text), ctx);
//   }
// };

std::ostream& operator<<(std::ostream& str, Token t) {
    str << "Token " << t.type << "," << " " << "`" << t.text << "`";
    return str;
}


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

    std::array<std::tuple<std::string, TokenType>, MACRO_SEP_COUNT> _separators;

    TokenType _lastKwd;

    Tokenizer(std::string_view text): 
        _srcText{text},  
        _tokIndex{0},
        _cur{0, 0, 0},
        _errBit{false},
        _state{State::START} {

        _separators = {
            #define X(str, tok) std::make_tuple((str), (TokenType::tok)),
            MACRO_SEPARATORS
            #undef X
        };

        std::sort(_separators.begin(), _separators.end(), [](const auto& lhs, const auto& rhs) {
            return std::get<0>(lhs).size() > std::get<0>(rhs).size();
        });
    }

    Tokenizer(const Tokenizer &o) = delete;
    Tokenizer(Tokenizer &&o) = delete;
    Tokenizer& operator = (const Tokenizer &o) = delete;
    Tokenizer& operator = (Tokenizer &&o) = delete;


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
        for (auto &[str, tokType]: _separators) {
            if (is_cur_matches(str)) {
                _lastSep = str;
                _lastSepType = tokType;
                return true;
            }
        }
        return false;
    }

    std::string_view slice_self(size_t l, size_t r) {
        assert(l < r && r <= _srcText.size());
        return slice(_srcText, l, r);
    }

    bool check_keyword(std::string_view word) {
        #define X(str, tp) if (word == str) { _lastKwd = TokenType::tp; return true; }
            MACRO_KWDS
        #undef X
        return false;
    }


    bool is_word_char(char c) {
        return isalpha(c) || c == '_';
    }

    size_t manage_state() {
        switch (_state) {
        case State::START:
            if (reached_end()) {}
            else if (cur_char() == '.') _state = State::DOT;
            else if (cur_char() == '\'') _state = State::SINGLE_QUOTE;
            else if (cur_char() == '"') _state = State::DOUBLE_QUOTE;
            else if (is_word_char(cur_char())) _state = State::WORD;
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
                add_token_and_reset(TokenType::CHAR, _startCur, slice_self(_startCur.pos+1, _cur.pos));
            }
            break;

        case State::DOUBLE_QUOTE:
            if (reached_end()) _state = State::INVALID;
            else if (cur_char() == '\\') return 2;
            else if (cur_char() == '\"') {
                add_token_and_reset(TokenType::STRING, _startCur, slice_self(_startCur.pos+1, _cur.pos));
            }
            break;

        case State::WORD:
            if (reached_end() || lookup_for_sep() || isspace(cur_char())) { 
                if (check_keyword(slice_self(_startCur.pos, _cur.pos))) {
                    add_token_and_reset(_lastKwd, _startCur, slice_self(_startCur.pos, _cur.pos));
                }
                else {
                    add_token_and_reset(TokenType::WORD, _startCur, slice_self(_startCur.pos, _cur.pos)); 
                }
                return 0;
            }
            else if (isalnum(cur_char()) || cur_char() == '_') {}
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
        case State::INVALID: 
            break;
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
