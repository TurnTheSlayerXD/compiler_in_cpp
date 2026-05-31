
#include "tokenizer.c"

#include <stdarg.h>
#include <macros.h>

#include "ast_types.h"

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





bool h_word_token( Queue *q, Token t ) {
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

bool h_op_token( Queue *q, Token t ) {
    
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

bool h_open_brace_token( Queue *q, Token t ) {
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

    return false;
}

bool h_close_brace_token( Queue *q, Token t) {
    (void) t;
    if (node_type_at(q, -2) == START_BRACKET) {
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


bool h_comma_token( Queue *q, Token t ) {
    
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

bool h_call_start( Queue *q, Token t ) {
    

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

bool h_call_end( Queue *q, Token t) {
    
    
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
