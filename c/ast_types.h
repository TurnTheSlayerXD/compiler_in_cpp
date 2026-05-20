#ifndef H_AST_TYPES

#define H_AST_TYPES

#define PREP_ENUM_OP \
	X(OP_PLUS, "+") \
	X(OP_MINUS, "-") \
	X(OP_MUL, "*") \
	X(OP_DIV, "/") \
	X(OP_PERCENT, "%")

#define cast(ptr, type) ((ptr)->_.type)


#define STATE_TO_H \
	X(ST_INSIDE_FUNC,  ((Handler[]){{T_CUSTOM_WORD, h_word_token }, \
                                 {T_NONE, h_op_token, is_op_token }, \
                                 {T_L_BR, h_open_brace_token }, \
                                 {T_COMMA, h_comma_token  }, \
                                 {T_L_BR, h_call_start }, \
                                 })) \
	X(ST_OPEN_BRACE,   ((Handler[]){{T_CUSTOM_WORD, h_word_token,}, \
                                 {T_NONE, h_op_token, is_op_token }, \
                                 {T_L_BR, h_open_brace_token }, \
                                 {T_R_BR, h_close_brace_token }, \
                                 {T_COMMA, h_comma_token }, \
                                 {T_L_BR, h_call_start }, \
                                 })) \
    X(ST_CALL_START,   ((Handler[]){{T_CUSTOM_WORD, h_word_token }, \
                                 {T_NONE, h_op_token, is_op_token }, \
                                 {T_L_BR, h_open_brace_token }, \
                                 {T_R_BR, h_close_brace_token }, \
                                 {T_COMMA, h_comma_token }, \
                                 {T_L_BR, h_call_start }, \
                                 {T_R_BR, h_call_end }, \
                                 }))

typedef enum _EnumOp {
    #define X(name, value) name,
		PREP_ENUM_OP
	#undef X
} EnumOp;



typedef struct _AstNode AstNode;

typedef struct _Word {
    String_View text;
} Word;

typedef struct _Statement {
    AstNode* expr;
} Statement;

typedef struct _Expr {
    EnumOp op;
    AstNode *lhs;
    AstNode *rhs;
} Expr;

typedef struct _Call {
    AstNode* callable;

    AstNode* args[10];
    int count;
} Call;

typedef enum _NodeType {
    NONE,
    WORD,
    STATEMENT,

    EXPR,
    BRACE_EXPR,
    CALL_EXPR,

    START_BRACKET,
    START_CALL,

} NodeType;

struct _AstNode {
    Tag tag;
    NodeType type;
	union {
        struct _Word Word;
        // contains ref to expression, ends with semicolon
        struct _Statement Statement;
        // contains seq of operators (math expression)
        struct _Expr Expr;

        struct _Call Call;
	} _;
};


typedef struct _Queue {
    int size;
    AstNode* nodes[30];
} Queue;


typedef enum _ParserState ParserState;

typedef bool (*Handler_Fun)(ParserState *e, Queue *q, Token t );
typedef bool (*Check_Op)(Token t);

typedef struct {
    enum EnumToken input;
    Handler_Fun handler;
    Check_Op check;
} Handler;


enum _ParserState {
    #define X(name, value) name,
        STATE_TO_H
    #undef X
};


const char* to_string_EnumOp(EnumOp op) {
    switch(op) {
        #define X(name, value) case name: return value;
            PREP_ENUM_OP
        #undef X
        default: assert(false && "UNREACHABLE"); return "";
    }
}



#endif
