
#include "tokenizer.c"

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

typedef enum _NodeType {
    NONE,
    WORD,
    STATEMENT,
    EXPR,
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
	} _;
};

#define cast(ptr, type) ((ptr)->_.type)

typedef struct _Queue {
    int size;
    AstNode* nodes[20];
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

NodeType last_node_type(Queue *queue) {
    return node_type_at(queue, -1);
}


struct {
    AstNode heap[20];
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

    return get_order(cast(lhs, Expr).op) -  get_order(cast(rhs, Expr).op);
}

typedef bool (*Handler)(Queue *q, Token t);

bool is_operand(AstNode *n) {
    switch (n->type) {
        case WORD: return true;
        default: return false;
    } 
}

bool handle_word_token(Queue *q, Token t) {
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



bool handle_op_token(Queue *q, Token t) {
    if (!is_op_token(t)) {
        return false;
    }
    if (node_type_at(q, -1) == WORD) {
        AstNode *expr = AstNode_new_Expr(t, t.tag);
        AstNode *word = pop_queue(q);
        cast(expr, Expr).lhs = word;    
        push_queue(q, expr);
        return true;  
    }
    if (node_type_at(q, -1) == EXPR) {
        AstNode *new_expr = AstNode_new_Expr(t, t.tag);
        AstNode *prev_expr = node_at(q, -1);
        
        if (node_cmp_expr(new_expr, prev_expr) < 0) {
            pop_queue(q);
            cast(new_expr, Expr).lhs = prev_expr;
            push_queue(q, new_expr);
            return true;
        }
        else {
            AstNode *parent = prev_expr;
            while (cast(prev_expr, Expr).rhs && isinstance(cast(prev_expr, Expr).rhs, EXPR) && node_cmp_expr(new_expr, prev_expr) >= 0) {
                parent = prev_expr;
                prev_expr = cast(prev_expr, Expr).rhs;
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

            return true;
        }
    }
    else {
        assert(false && "TODO");
    }

    return false;
}


typedef struct _AstPrintParams {
    int ind_step;
    const char* sep;
} AstPrintParams;

char* to_string_AstTree(AstPrintParams params, char* ptr, AstNode *n, int ind) {
    if (!n) {
        for (int i = 0; i < ind; ++i) { *ptr = ' '; ++ptr;}
        int ret = sprintf(ptr, "%s,%s", "NULL", params.sep);
        return ptr + ret;
    }
    switch (n->type) {
        case WORD: 
            for (int i = 0; i < ind; ++i) { *ptr = ' '; ++ptr;}
            int ret = sprintf(ptr, "%s [%.*s],%s", "WORD", cast(n, Word).text.len, cast(n, Word).text.ptr, params.sep); 
            return ptr + ret;
        case EXPR: 
            for (int i = 0; i < ind; ++i) { *ptr = ' '; ++ptr;}
            int c = sprintf(ptr, "%s [%s]: {%s", "EXPR", to_string_EnumOp(cast(n, Expr).op), params.sep); 
            ptr = to_string_AstTree(params, ptr + c, cast(n, Expr).lhs, ind + params.ind_step);
            ptr = to_string_AstTree(params, ptr, cast(n, Expr).rhs, ind + params.ind_step);
            for (int i = 0; i < ind; ++i) { *ptr = ' '; ++ptr;}
            c = sprintf(ptr, "}%s", params.sep); 
            return ptr + c;
        default:
            assert(false && "to_string_AstTree Unknown AstNode Type");
            return NULL;
    }
}
