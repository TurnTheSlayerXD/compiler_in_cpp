#include "ast.c"

typedef struct _Parser {
    ParserState state;
} Parser;


Parser Parser_new(void) {

    return (Parser){0};
}


AstNode* parse(Parser *p, Tokenizer *tokenizer) {
    bool handled = false;
    Token *t;
    Queue q = {0};
	while ((t = tok_next_token(tokenizer))) {
        switch (p->state) {
            #define X(name, value) \
            case name: \
                handled = false; \
                Handler *h_arr = value; \
                for (int h = 0; h < COUNT_OF(value); ++h) \
                { \
                    if (h_arr[h](&p->state, &q, *t)) {\
                        handled = true; \
                        break; \
                    } \
                } \
                if (!handled) assert(false && "couldn't handle state"); \
                break;
            #undef X
        } 
    }

    return NULL;
}





int main(void) {
	
    const char *program_text = "aboba * (ptr + x)() + (asd)() + x";

	Tokenizer tokenizer = Tokenizer_new((String_View){.ptr = program_text, .len = strlen(program_text)});

    Handler handlers[] = {
        h_word_token, 
        h_op_token,
        h_brace_token,
        h_comma_token,
        h_call_start,
        h_call_end,
    };

    Queue q = {0};

    Token *t;

    Parser p = Parser_new();

	while ((t = tok_next_token(&tokenizer))) {
        bool handled = false;
        for (int h = 0; h < COUNT_OF(handlers); ++h) {
            if (handlers[h](&p.state, &q, *t)) {
                handled = true;
                break;
            }
        }

        if (!handled) {
            assert(false && "FAILED handling TOKEN");
        }
	}


    AstPrintBuffer buf = {0};
    to_string_AstTree(
        (AstPrintParams){.ind_step = 3, .sep = "\n\r"}, 
        &buf,
        node_at(&q, -1)
    );

    printf("_____________________________________________\n");
    printf("%s\n", buf.ptr);
    printf("_____________________________________________\n");

    clear_AstPrintBuffer(&buf);

    clear_Token_array(&tokenizer.tokens);
    return 0;
}
