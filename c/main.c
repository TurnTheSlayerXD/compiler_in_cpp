

#include "ast.c"


int main(void) {
	
    const char *program_text = "a + x * c / m + d - e / t * y + r / k";

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
    to_string_AstTree((AstPrintParams){.ind_step = 1, .sep = ""}, buf, node_at(&q, -1), 0);
    printf("_____________________________________________\n");
    printf("%s", buf);
    printf("\n_____________________________________________\n");

    clear_Token_array(&tokenizer.tokens);
    return 0;
}
