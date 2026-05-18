
#include "../ast.c"
#include <string.h>

void test(const char *test_id, const char *program_text, const char *expected_ast, bool debug) {
    printf("_____________________________________________\n");
	
    Tokenizer tokenizer = Tokenizer_new((String_View){.ptr = program_text, .len = strlen(program_text)});

    Handler handlers[] = {
        handle_word_token, 
        handle_op_token,
        handle_brace_token,
    };

    Queue q = {0};

    Token *t;
	while ((t = tok_next_token(&tokenizer))) {
        bool handled = false;
        for (int h = 0; h < COUNT_OF(handlers); ++h) {
            if (handlers[h](&q, *t, &tokenizer)) {
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
        (AstPrintParams){.ind_step = 0, .sep = ""}, 
        &buf,
        node_at(&q, -1)
    );

    if (strcmp(expected_ast, buf.ptr) != 0) {
        fprintf(stderr, 
            "FAILED TEST %s!\n"
            "Expected:\n\r%s\n"
            "Found:\n\r%s\n", test_id, expected_ast, buf.ptr);
    }
    else {
        printf("SUCCESFUL TEST %s!\n", test_id);
    }

    if (debug) {
        to_string_AstTree(
            (AstPrintParams){.ind_step = 4, .sep = "\n\r"}, 
            &buf,
            node_at(&q, -1)
        );
        printf("%s\n", buf.ptr);
    }

    printf("\n_____________________________________________\n\n");

    clear_AstPrintBuffer(&buf);
    clear_Token_array(&tokenizer.tokens);
}



int main(void) {
	
    test(
        "1",

        "a + x * c / m + d - e / t * y + r / k", 

        "EXPR [+]: {WORD [a],EXPR [+]: {EXPR [/]: {EXPR [*]: {WORD [x],WORD [c],}WORD [m],}EXPR [+]: {EXPR [-]: {WORD [d],EXPR [*]: {EXPR [/]: {WORD [e],WORD [t],}WORD [y],}}EXPR [/]: {WORD [r],WORD [k],}}}}",

        true
    );


    
    test(
        "2",
        "g * (a + b) / (k - f) + c * d * (x + z + (k - koaf))",

        "EXPR [+]: {EXPR [/]: {EXPR [*]: {WORD [g],EXPR [+]: {WORD [a],WORD [b],}}EXPR [-]: {WORD [k],WORD [f],}}EXPR [*]: {EXPR [*]: {WORD [c],WORD [d],}EXPR [+]: {EXPR [+]: {WORD [x],WORD [z],}EXPR [-]: {WORD [k],WORD [koaf],}}}}",

        true
    );


    return 0;
}
