#ifndef TOKENS_DEF_HPP
#define TOKENS_DEF_HPP

#include <iostream>
#include <iostream>
#include <cctype>
#include <optional>
#include <vector>
#include <algorithm>
#include <cassert>
#include <print>
#include <format>

#define MACRO_SEPARATORS \
    X("=",  ASSIGN   )\
    X("+",  PLUS     )\
    X("-",  MINUS    )\
    X("*",  ASTERISC )\
    X("/",  DIV      )\
    X("&",  ADDR     )\
    X("%",  PERCENT  )\
    X("++", INCR     )\
    X("--", DECR     )\
    X(">",  GR       )\
    X("<",  LE       )\
    X(">=", GR_E     )\
    X("<=", LE_E     )\
    X("==", EQ       )\
    X("&&", AND      )\
    X("||", OR       )\
    X(";",  SEMICOLON)\
    X(".",  DOT      )\
    X(",",  COMMA    )\
    X("(",  L_BR     )\
    X(")",  R_BR     )\
    X("[",  L_SUBSCR )\
    X("]",  R_SUBSCR )\
    X("{",  L_CURL   )\
    X("}",  R_CURL   )

#define MACRO_KWDS\
    X("const",    KWD_CONST   )\
    X("return",   KWD_RET     )\
    X("struct",   KWD_STRUCT  )\
    X("for",      KWD_FOR     )\
    X("while",    KWD_WHILE   )\
    X("break",    KWD_BREAK   )\
    X("continue", KWD_CONTINUE)\
    X("do",       KWD_DO      )\
    X("if",       KWD_IF      )\
    X("else",     KWD_ELSE    )\
    X("switch",   KWD_SWITCH  )\
    X("case",     KWD_CASE    )

enum class TokenType {
    ASSIGN,
    PLUS,
    MINUS,
    ASTERISC,
    DIV,
    ADDR,
    INCR,
    DECR,
    PERCENT,
    GR,
    LE,
    GR_E,
    LE_E,
    EQ,
    AND,
    OR,
    LIT_CHAR,
    LIT_STRING,
    WORD,
    LIT_INT,
    LIT_FLOAT,
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
    THREE_DOTS,

    EOF_TOK,
};

constexpr auto MACRO_SEP_COUNT = std::size({
    #define X(str, tok) TokenType::tok,
        MACRO_SEPARATORS
    #undef X
});


std::string_view to_string(TokenType t) {
    using enum TokenType;
    switch(t) {
        case PLUS         : return "`+`";
        case MINUS        : return "`-`";
        case ASTERISC     : return "`*`";
        case DIV          : return "`/`";
        case PERCENT      : return "%";
        case L_BR         : return "`(`";
        case R_BR         : return "`)`";
        case WORD         : return "`word`";
        case LIT_INT      : return "`num INT`";
        case LIT_FLOAT    : return "`num FLOAT`";
        case LIT_CHAR     : return "`char`";
        case LIT_STRING   : return "`string`";
        case COMMA        : return "`,`";
        case L_SUBSCR     : return "`[`";
        case R_SUBSCR     : return "`]`";
        case ASSIGN       : return "`=`";
        case INCR         : return "`++`";
        case DECR         : return "`--`";
        case GR           : return "`>`";
        case LE           : return "`<`";
        case GR_E         : return "`>=`";
        case LE_E         : return "`<=`";
        case EQ           : return "`==`";
        case AND          : return "`&&`";
        case OR           : return "`||`";
        case ADDR         : return "`&`";
        case KWD_CONST    : return "`const`";
        case KWD_RET      : return "`return`";
        case KWD_STRUCT   : return "`struct`";
        case KWD_FOR      : return "`for`";
        case KWD_WHILE    : return "`while`";
        case KWD_BREAK    : return "`break`";
        case KWD_CONTINUE : return "`continue`";
        case KWD_DO       : return "`do`";
        case KWD_IF       : return "`if`";
        case KWD_ELSE     : return "`else`";
        case KWD_SWITCH   : return "`switch`";
        case KWD_CASE     : return "`case`";
        case SEMICOLON    : return "`;`";
        case L_CURL       : return "`{`";
        case R_CURL       : return "`}`";
        case THREE_DOTS   : return "...";
        default           : { fprintf(stderr, "\nTo_string not implemented for: [%d]\n", static_cast<int>(t)); assert(false && "UNREACHABLE"); return "";}
    }
}

#endif
