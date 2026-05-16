
#include "../ast.c"

void test(const char *program_text, const char *expected_ast, bool debug) {
    printf("_____________________________________________\n");
	
    Tokenizer tokenizer = Tokenizer_new((String_View){.ptr = program_text, .len = strlen(program_text)});

    Handler handlers[] = {
        handle_word_token, 
        handle_op_token,
    };

    Queue q = {0};

    Token *t;
	while ((t = tok_next_token(&tokenizer))) {
        bool handled = false;
        for (int h = 0; h < COUNT_OF(handlers); ++h) {
            if (handlers[h](&q, *t)) {
                handled = true;
                break;
            }
        }

        if (!handled) {
            assert(false && "FAILED handling TOKEN");
        }
	}

    char buf[300] = {0};
    to_string_AstTree((AstPrintParams){ .sep = "", .ind_step = 0 }, buf, node_at(&q, -1), 0);



    if (strcmp(expected_ast, buf) != 0) {
        printf("FAILED TEST %s!\n", test_id);
    }
    else {
        printf("SUCCESFUL TEST %s!\n", test_id);
    }

    if (debug) {
        memset(buf, COUNT_OF(buf), 0);
        to_string_AstTree((AstPrintParams){ .sep = "\n\r", .ind_step = 4 }, buf, node_at(&q, -1), 0);
    }

    printf("\n_____________________________________________\n\n");

    clear_Token_array(&tokenizer.tokens);
}



int main(void) {
	
    test(
        "a + x * c / m + d - e / t * y + r / k", 
        "EXPR [+]: { WORD [a], EXPR [+]: {  EXPR [/]: {   EXPR [*]: {    WORD [x],    WORD [c],   }   WORD [m],  }  EXPR [+]: {   EXPR [-]: {    WORD [d],    EXPR [*]: {     EXPR [/]: {      WORD [e],      WORD [t],     }     WORD [y],    }   }   EXPR [/]: {    WORD [r],    WORD [k],   }  } }}",
        true
    );


    return 0;
}
