

#include "ast.c"



int main(void) {
	
    const char *program_text = "aboba * (ptr + x)() + (asd)() + x";

	Tokenizer tokenizer = Tokenizer_new((String_View){.ptr = program_text, .len = strlen(program_text)});

    Handler handlers[] = {
        handle_word_token, 
        handle_op_token,
        handle_brace_token,
        handle_comma_token,
        
        handle_fun_call_start,
        handle_fun_call_end,
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
