#include "tokenizer.c"


enum EnumOpNode {
    OP_PLUS,
    OP_MINUS,
    OP_MUL,
    OP_DIV,
    OP_PERCENT,
};


typedef struct AstNode {

    Tag pos;
    bool is_partial;

    enum EnumNode {
        NAN,
        WORD,
        
        STATEMENT,
        
        EXPR,

        FUN_CALL,
    } type;

	union {
        // contains ref to expression, ends with semicolon
        struct {
            AstNode* expr;
        } statement;
        
        // contains seq of operators (math expression)
        struct {
            EnumOpNode op;
            AstNode *lhs;
            AstNode *rhs;
        } expr;
	} _;

} AstNode;

typedef struct Queue {
    int size;
    AstNode* nodes[10];
};

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
    return q->ptr[q->size];
} 

AstNode* node_at(Queue *queue, int i) {
    if (i >= 0) {
        assert(false && "get_node_queue index can be only less than zero");
    }
    if (queue->size + i < 0) {
        assert(false && "get_node_queue index out of bounds");
    }

    return queue->ptr[queue->size - i];
} 

EnumNode node_type_at(Queue *queue, int i) {
    if (i >= 0) {
        assert(false && "get_node_queue index can be only less than zero");
    }
    if (queue->size + i < 0) {
        return NAN;
    }
    return queue->ptr[queue->size + i];
}

EnumNode last_node_type(Queue *queue, int i) {
    return node_type_at(queue, -1);
}


struct {
    AstNode heap[10];
    AstNode size = 0;
} __nodeHeap = {0};

AstNode* AstNode_new(enum EnumAstNode type, Tag tag) {
    if (__nodeHeap.size >= COUNT_OF(__nodeHeap.heap)) {
        assert(false && "Exceeded limit of node heap");
    }
    __nodeHeap.size += 1;
    AstNode *p = &__nodeHeap.heap[__nodeHeap.size - 1];
    p->type = type;
    p->tag = tag;
    p->_ = {0};
    return p; 
}

int op_order(EnumOpNode t) {
    switch (f) {
        case OP_PLUS: case OP_MINUS: case OP_PERCENT: return 0;
        case OP_MUL: case OP_DIV: return 1;
        default: assert(false && "op_node UNREACHABLE"); return 0;
    }    
}

int nodes_cmp(AstNode *prev_node, AstNode *new_node) {
    return op_order(l) - op_order(r); 
}

AstNode* handle_token_word(Queue *queue, Token t) {
    if (t.type != T_CUSTOM_WORD) {
        return NULL;
    }
    
    if (node_type_at(queue, -1) == EXPR) {
        AstNode *expr = pop_queue(q);
        expr->_.rhs = AstNode_new(WORD, false);
        return expr;
    }

    return AstNode_new(WORD, false);
}

#define n_f(node, field) node->_.field

AstNode* handle_op_token(Queue *q, Token t) {
    
    switch(t.type) {
        #define X(name, str) case name: break;
            TOKENS_OPERATIONS
        #undef X
        default:  return NULL;
    }

    if (node_type_at(q, -1) == WORD) {
        AstNode *new_node = AstNode_new(EXPR, true);
        AstNode *word = pop_queue(q);
        new_node->_.lhs = word;    
        return new_node;  
    }

    if (node_type_at(q, -1) == EXPR) {
        AstNode *new_expr = AstNode_new(EXPR, false);
        AstNode *prev_expr = pop_queue(q);
        int cmp = nodes_cmp(prev_expr, new_expr);
        if (cmp <= 0) {
            n_f(new_expr, lhs) = n_f(prev_expr, rhs); 
            n_f(prev_expr, rhs) = new_expr;  
            push_queue(prev_expr);
            return new_expr;
        }
        else {
            n_f(new_expr, lhs) = prev_expr;
            return new_expr;
        }
    }
    else if (node_type_at(q, -1) == STATEMENT) {
        AstNode* statement = pop_queue(q);
        n_f(statement, expr) = AstNode_new(EXPR, false); 
        push_queue(statement);
    }
  
}

AstNode* handle_brace_token(Queue *q, Token t) {
    switch(t) { case T_L_BR : case T_R_BR: break; default: return NULL; }


    if (t == T_L_BR) {

        if (node_type_at(q, -1) == EXPR 
         && n_f(node_at(q, -1), rhs) 
         && n_f(node_at(q, -1), rhs).type == WORD
        ) {
            AstNode *fun_call = AstNode_new(FUN_CALL);
            AstNode *word = n_f(node_at(q, -1), rhs);
            n_f(fun_call, fun_name) = word;
            n_f(node_at(q, -1), rhs) = fun_call;
            return fun_call;
        }
        else if (node_type_at(q, -1) == EXPR 
              && !n_f(node_at(q, -1), rhs)) {
            AstNode *word = pop_queue(q);
            AstNode *prev_expr = pop_queue(q);
            prev_expr->tag = 
        }
     
        return AstNode_new(EXPR, false);
    }

}


int main() {
	
    const char *program_text = "1 + x * 69 == 35";

	Tokenizer tokenizer = Tokenizer_new(program_text);
	AstNode *top_level;



    Token *t;
	while (t = tok_next_token(tokenizer)) {
        
	}


    clear_Token_array(&tokenizer.tokens);
    return 0;
}