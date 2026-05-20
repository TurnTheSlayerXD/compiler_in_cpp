#ifndef H_AST_PRINT
#define H_AST_PRINT

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



#endif