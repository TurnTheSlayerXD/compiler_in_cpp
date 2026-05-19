
#include "tokenizer.c"

#include <stdarg.h>
#include <macros.h>

#define PREP_ENUM_OP \
	X(OP_PLUS, "+") \
	X(OP_MINUS, "-") \
	X(OP_MUL, "*") \
	X(OP_DIV, "/") \
	X(OP_PERCENT, "%")

typedef enum _EnumOp {
    #define X(name, value) name,
		PREP_ENUM_OP
	#undef X
} EnumOp;


#define STATE_TO_H \
	X(INSIDE_FUNC, ((Handler[]){ h_word_token, h_op_token, h_brace_token, h_comma_token, h_call_start, h_call_end }))

typedef enum _ParserState {
    #define X(name, value) name,
        STATE_TO_H
    #undef X
} ParserState;


const char* to_string_EnumOp(EnumOp op) {
    switch(op) {
        #define X(name, value) case name: return value;
            PREP_ENUM_OP
        #undef X
        default: assert(false && "UNREACHABLE"); return "";
    }
}

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

#define cast(ptr, type) ((ptr)->_.type)

typedef struct _Queue {
    int size;
    AstNode* nodes[30];
} Queue;

void push_queue(Queue *queue, AstNode *node) {
    if (queue->size >= COUNT_OF(queue->nodes)) {
        assert(false && "Exceeded limit count of queue");
    }   
    queue->nodes[queue->size] = node;
    queue->size += 1;
}

AstNode* pop_queue(Queue *q) {
    if (q->size <= 0) {
        assert(false && "Popping out of empty queue");
    }
    q->size -= 1;
    return q->nodes[q->size];
} 

AstNode* node_at(Queue *queue, int i) {
    if (i >= 0) {
        assert(false && "get_node_queue index can be only less than zero");
    }
    if (queue->size + i < 0) {
        assert(false && "get_node_queue index out of bounds");
    }

    return queue->nodes[queue->size + i];
} 

NodeType node_type_at(Queue *queue, int i) {
    if (i >= 0) {
        assert(false && "get_node_queue index can be only less than zero");
    }
    if (queue->size + i < 0) {
        return NONE;
    }
    return queue->nodes[queue->size + i]->type;
}

struct {
    AstNode heap[50];
    int size;
} __nodeHeap = {0};

AstNode* AstNode_new(NodeType type, Tag tag) {
    if (__nodeHeap.size >= COUNT_OF(__nodeHeap.heap)) {
        assert(false && "Exceeded limit of node heap");
    }
    __nodeHeap.size += 1;
    AstNode *p = &__nodeHeap.heap[__nodeHeap.size - 1];
    p->type = type;
    p->tag = tag;
    memset(&p->_, 0, sizeof(p->_));

    return p; 
}


AstNode* AstNode_new_Word(String_View text, Tag tag) {
    AstNode *p = AstNode_new(WORD, tag);
    cast(p, Word).text = text;
    return p; 
}

EnumOp cast_token_to_enum_op(Token t) {
    switch (t.type) {
        case T_OP_PLUS: return OP_PLUS;
        case T_OP_MINUS: return OP_MINUS;
        case T_STAR: return OP_MUL;
        case T_OP_DIVIDE: return OP_DIV;
        case T_OP_PERCENT: return OP_PERCENT;
        
        default: assert(false && "Unexpected token to enum cast"); return OP_PERCENT;
    }
}

AstNode* AstNode_new_Expr(Token t, Tag tag) {
    AstNode *p = AstNode_new(EXPR, tag);
    cast(p, Expr).op = cast_token_to_enum_op(t);
    cast(p, Expr).lhs = NULL;
    cast(p, Expr).rhs = NULL;
    return p; 
}



bool is_op_token(Token t) {
    switch(t.type) {
        #define X(name, str) case name: return true;
            TOKENS_OPERATIONS
        #undef X
        case T_STAR: return true;
        default:  return false;
    }
}

bool isinstance(AstNode *n, NodeType t) {
    return n->type == t;
}


int get_order(EnumOp op) {
    switch(op) {
        case OP_PLUS: case OP_PERCENT: return 0;
        case OP_MINUS: return 1;
        case OP_MUL: case OP_DIV: return 2;
        default: assert(false && "Unknown op passed"); return -1;
    }
}

int node_cmp_expr(AstNode *lhs, AstNode *rhs) {
    if (!isinstance(lhs, EXPR) || !isinstance(rhs, EXPR)) {
        assert(false && "Expected both args of node_cmp_expr to be EXPR");
    }

    return get_order(cast(lhs, Expr).op) - get_order(cast(rhs, Expr).op);
}


bool is_operand(AstNode *n) {
    switch (n->type) {
        case WORD: return true;
        default: return false;
    } 
}


typedef struct _AstPrintParams {
    int ind_step;
    const char* sep;
} AstPrintParams;

typedef struct _AstPrintBuffer {
    char *ptr; int size;
} AstPrintBuffer;

AstPrintBuffer* __print_buffer; 
AstPrintParams __print_params; 

void __realloc_print_buffer(char **ptr, int add_size) {
    if (add_size < 0) {
        assert(false && "Unexpected add_size");
    }
    if (add_size == 0) {
        return;
    }
    if (*ptr + add_size + 1 >= __print_buffer->ptr + __print_buffer->size) {
        size_t dif = *ptr - __print_buffer->ptr;
        __print_buffer->ptr = realloc(__print_buffer->ptr, __print_buffer->size + add_size * 2 + 1);
        __print_buffer->size += add_size * 2 + 1;
        *ptr = __print_buffer->ptr + dif;
    }

    if (__print_buffer->size > 4000) {
        assert(false);
    }
}

void clear_AstPrintBuffer(AstPrintBuffer* buf) {
    if (!buf) {
        return;
    }
    free(buf->ptr);
    buf->ptr = NULL;
    buf->size = 0;
}

char* __write_with_indentation(char *ptr, int ind, const char *fmt, ...) {
    __realloc_print_buffer(&ptr, ind);
    for (int i = 0; i < ind; ++i) { *ptr = ' '; ++ptr; }

    va_list args1;

    va_start(args1, fmt);
    
    va_list args2;
    va_copy(args2, args1);

    int chars_to_be_written = vsnprintf(NULL, 0, fmt, args1);
    __realloc_print_buffer(&ptr, chars_to_be_written);

    va_end(args1);

    ptr += vsnprintf(ptr, chars_to_be_written + 1, fmt, args2);

    va_end(args2);

    return ptr;
}

char *__to_string_AstTree(char* ptr, AstNode *n, int ind) {
    if (!n) {
        __realloc_print_buffer(&ptr, ind);
        for (int i = 0; i < ind; ++i) { *ptr = ' '; ++ptr; }

        int chars_to_be_written = snprintf(NULL, 0, "%s,%s", "NULL", __print_params.sep);
        __realloc_print_buffer(&ptr, chars_to_be_written);
        ptr += sprintf(ptr, "%s,%s", "NULL", __print_params.sep);
        return ptr;
    }
    switch (n->type) {
        case WORD: 
            __realloc_print_buffer(&ptr, ind);
            for (int i = 0; i < ind; ++i) { *ptr = ' '; ++ptr; }
            int chars_to_be_written = snprintf(NULL, 0, "%s [%.*s],%s", "WORD", cast(n, Word).text.len, cast(n, Word).text.ptr, __print_params.sep);

            __realloc_print_buffer(&ptr, chars_to_be_written);
            ptr += sprintf(ptr, "%s [%.*s],%s", "WORD", cast(n, Word).text.len, cast(n, Word).text.ptr, __print_params.sep); 
            return ptr;

        case EXPR: case BRACE_EXPR: 
            __realloc_print_buffer(&ptr, ind);
            for (int i = 0; i < ind ; ++i) { *ptr = ' '; ++ptr; }
            
            chars_to_be_written = snprintf(NULL, 0, "%s [%s]: {%s", "EXPR", to_string_EnumOp(cast(n, Expr).op), __print_params.sep);
            __realloc_print_buffer(&ptr, chars_to_be_written);
            ptr += sprintf(ptr, "%s [%s]: {%s", "EXPR", to_string_EnumOp(cast(n, Expr).op), __print_params.sep); 

            ptr = __to_string_AstTree(ptr, cast(n, Expr).lhs, ind + __print_params.ind_step);
            ptr = __to_string_AstTree(ptr, cast(n, Expr).rhs, ind + __print_params.ind_step);
            
            __realloc_print_buffer(&ptr, chars_to_be_written);
            for (int i = 0; i < ind; ++i) { *ptr = ' '; ++ptr;}
        
            chars_to_be_written = snprintf(NULL, 0, "}%s", __print_params.sep); 
            __realloc_print_buffer(&ptr, chars_to_be_written);
            ptr += sprintf(ptr, "}%s", __print_params.sep); 
            return ptr;
        
        case CALL_EXPR: 
            ptr = __write_with_indentation(ptr, ind, "%s: {%s", "Call expr", __print_params.sep);
            ptr = __write_with_indentation(ptr, ind + __print_params.ind_step, "%s: {%s", "callable",  __print_params.sep);
            ptr = __to_string_AstTree(ptr, cast(n, Call).callable, ind + 2 * __print_params.ind_step);
            ptr = __write_with_indentation(ptr, ind + __print_params.ind_step, "},%s",  __print_params.sep);

            ptr = __write_with_indentation(ptr, ind + __print_params.ind_step, "%s: [%s", "arguments", __print_params.sep);

            for (int i = 0; i < cast(n, Call).count; ++i) {
                ptr = __to_string_AstTree(ptr, cast(n, Call).args[i], ind + 2 * __print_params.ind_step);
            }

            ptr = __write_with_indentation(ptr, ind + __print_params.ind_step, "],%s", __print_params.sep);

            return ptr;

        default:
            assert(false && "to_string_AstTree Unknown AstNode Type");
            return NULL;
    }
}

void to_string_AstTree(AstPrintParams params, AstPrintBuffer* buf, AstNode *n) {
    if (!buf) {
        assert(false && "AstPrintBuffer = NULL");
    }
    clear_AstPrintBuffer(buf);
    __print_buffer = buf;

    *__print_buffer = (AstPrintBuffer){ .ptr = malloc(10), .size = 10 };
    __print_params = params;
    __to_string_AstTree(__print_buffer->ptr, n, 0);
}

typedef bool (*Handler)(ParserState *e, Queue *q, Token t );


bool h_word_token(ParserState *e, Queue *q, Token t ) {
    

    if (t.type != T_CUSTOM_WORD) {
        return false;
    }

    if (node_type_at(q, -1) == EXPR) {
        AstNode *expr = node_at(q, -1);
        while (cast(expr, Expr).rhs) {
            if (!isinstance(cast(expr, Expr).rhs, EXPR)) {
                break;
            }
            expr = cast(expr, Expr).rhs;
        }
        if (cast(expr, Expr).rhs) {
            fprintf(stderr, "Failed to find place for token at "FMT_TAG"\n", FMT_ARGS_TAG(t.tag));
            return false;
        }

        cast(expr, Expr).rhs = AstNode_new_Word(t.text, t.tag);
        return true;
    }

    push_queue(q, AstNode_new_Word(t.text, t.tag));
    return true;
}

bool h_op_token(ParserState *e, Queue *q, Token t ) {
    
    if (!is_op_token(t)) {
        return false;
    }
    
    if (node_type_at(q, -1) == EXPR) {
        AstNode *prev_expr = node_at(q, -1);
        AstNode *new_expr = AstNode_new_Expr(t, t.tag);
        if (node_cmp_expr(new_expr, prev_expr) < 0) {
            pop_queue(q);
            cast(new_expr, Expr).lhs = prev_expr;
            push_queue(q, new_expr);
        }
        else {
            AstNode *parent = prev_expr;
            while (cast(prev_expr, Expr).rhs && isinstance(cast(prev_expr, Expr).rhs, EXPR) && node_cmp_expr(new_expr, prev_expr) >= 0) {

                parent = prev_expr;
                prev_expr = cast(prev_expr, Expr).rhs;
            }

            if (parent == prev_expr) {
                if (node_cmp_expr(new_expr, parent) > 0) {
                    cast(new_expr, Expr).lhs = cast(prev_expr, Expr).rhs;
                    cast(parent, Expr).rhs = new_expr;
                }
                else {
                    pop_queue(q);
                    cast(new_expr, Expr).lhs = parent;
                    push_queue(q, new_expr);
                }

                return true;
            }

            if (!cast(prev_expr, Expr).rhs) {
                assert(false && "No place for op node insertion");
            }

            // if new_expr is * and prev_expr is +
            if (node_cmp_expr(new_expr, prev_expr) > 0) {
                cast(new_expr, Expr).lhs = cast(prev_expr, Expr).rhs;
                cast(prev_expr, Expr).rhs = new_expr;
            }
            // if new_expr is + and prev_expr is *
            else if (node_cmp_expr(new_expr, prev_expr) <= 0) {
                cast(parent, Expr).rhs = new_expr;
                cast(new_expr, Expr).lhs = prev_expr;
            }
        }
        return true;
    }
    if (node_type_at(q, -1) == START_BRACKET) {
        push_queue(q, AstNode_new_Expr(t, t.tag));
        return true;
    }
    if (node_type_at(q, -1) == WORD || node_type_at(q, -1) == CALL_EXPR) {
        AstNode *expr = AstNode_new_Expr(t, t.tag);
        AstNode *word = pop_queue(q);
        cast(expr, Expr).lhs = word;    
        push_queue(q, expr);
        return true;  
    }
    
    push_queue(q, AstNode_new_Expr(t, t.tag));

    return true;
}


bool is_leaf_type(NodeType n) {
    return n == CALL_EXPR || n == WORD || n == BRACE_EXPR;
}

bool h_brace_token(ParserState *e, Queue *q, Token t ) {
    

    switch (t.type) {
        case T_L_BR: case T_R_BR: break;
        default: return false;
    }

    if (t.type == T_L_BR) {
        
        if (is_leaf_type(node_type_at(q, -1))) {
            return false;
        }
        
        if (node_type_at(q, -1) == EXPR) {
            AstNode *expr = node_at(q, -1);
            while (cast(expr, Expr).rhs && isinstance(cast(expr, Expr).rhs, EXPR)) {
                expr = cast(expr, Expr).rhs;
            }
            if (cast(expr, Expr).rhs) {
                return false;
            }
        }

        push_queue(q, AstNode_new(START_BRACKET, t.tag));
        return true;
    }
    if (t.type == T_R_BR && node_type_at(q, -2) == START_BRACKET) {
        // Pop node, which can be either WORD or EXPR
        AstNode *inside_brace_node = pop_queue(q);

        if (isinstance(inside_brace_node, EXPR)) {
            inside_brace_node->type = BRACE_EXPR;
        }
        // Pop START_BRACKET node
        pop_queue(q);

        if (node_type_at(q, -1) == EXPR) {
            AstNode *expr = node_at(q, -1);
            // Trying to insert into existing EXPR NODE
            while (cast(expr, Expr).rhs && isinstance(cast(expr, Expr).rhs, EXPR)) {
                expr = cast(expr, Expr).rhs;
            }
            if (cast(expr, Expr).rhs) {
                return false;
            }
            cast(expr, Expr).rhs = inside_brace_node;
        }
        // Otherwise just PUSHING queue with expression inside BRACES
        else {
            push_queue(q, inside_brace_node);
        }

        return true;
    }
    return false;
}

void push_fun_arg(AstNode *fun, AstNode *arg) {
    if (!isinstance(fun, START_CALL)) {
        assert(false && "Expected fun node at push_fun_arg");
    }

    if (cast(fun, Call).count >= COUNT_OF(cast(fun, Call).args)) {
        assert(false && "Exceeded limit of fun->args COUNT");
    }

    cast(fun, Call).args[cast(fun, Call).count] = arg;
    cast(fun, Call).count += 1;
}


bool h_comma_token(ParserState *e, Queue *q, Token t ) {
    
    switch (t.type) {
        case T_COMMA: break;
        default: return false; 
    }

    if (node_type_at(q, -2) == START_CALL) {
        AstNode *fun = node_at(q, -2);
        AstNode *arg = pop_queue(q);
        push_fun_arg(fun, arg);
        return true;
    }

    return false;
}

AstNode* AstNode_newFunCall(AstNode *start_fun) {
    if (!isinstance(start_fun, START_CALL)) {
        assert(false && "Expected START_FUN_CALL in call AstNode_newFunCall");
    }
    start_fun->type = CALL_EXPR;
    return start_fun;  
}

bool h_call_start(ParserState *e, Queue *q, Token t ) {
    

    switch (t.type) {
        case T_L_BR: break;
        default: return false;
    }    

    if (node_type_at(q, -1) == EXPR) {
        AstNode *prev = node_at(q, -1);

        while (cast(prev, Expr).rhs && isinstance(cast(prev, Expr).rhs, EXPR)) {
            prev = cast(prev, Expr).rhs;
        }

        if (cast(prev, Expr).rhs && !isinstance(cast(prev, Expr).rhs, EXPR)) {

            AstNode* start_fun = AstNode_new(START_CALL, t.tag);
            cast(start_fun, Call).callable = cast(prev, Expr).rhs;
            cast(prev, Expr).rhs = NULL;

            push_queue(q, start_fun);

            return true;
        }

        return false;
    }

    if (node_type_at(q, -1) != NONE) {
        AstNode* start_fun = AstNode_new(START_CALL, t.tag);
        AstNode *callable = pop_queue(q);
        cast(start_fun, Call).callable = callable;
        push_queue(q, start_fun);
        return true;
    }
    
    return false;
}

bool h_call_end(ParserState *e, Queue *q, Token t) {
    
    
    switch (t.type) {
        case T_R_BR: break;
        default: return false;
    }

    if (node_type_at(q, -2) == START_CALL || node_type_at(q, -1) == START_CALL) {

        AstNode *start_fun = NULL;
        if (node_type_at(q, -1) != START_CALL) {
            AstNode *last_arg = pop_queue(q);
            start_fun = pop_queue(q);
            push_fun_arg(start_fun, last_arg);
        }
        else {
            start_fun = pop_queue(q);
        }
        
        AstNode *fun = AstNode_newFunCall(start_fun);

        if (node_type_at(q, -1) == EXPR) {
            AstNode *expr = node_at(q, -1);
            while (cast(expr, Expr).rhs && isinstance(cast(expr, Expr).rhs, EXPR)) {
                expr = cast(expr, Expr).rhs;
            }
            if (!cast(expr, Expr).rhs) {
                cast(expr, Expr).rhs = fun; 
            }
            else {
                push_queue(q, fun);
            }
        }
        else {
            push_queue(q, fun);
        }
            
        return true;
    }
   
    return false;
}