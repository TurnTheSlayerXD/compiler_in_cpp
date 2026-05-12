#include "tokenizer.c"





int main() {
	const char *program_text = "int val = 1;";
	enum EnumToken target[] = { T_KWD_TYPE_INT, T_CUSTOM_WORD, T_ASSIGN, T_LIT_I, T_SEMICOLON };
	
    Token_array actual = {0};
    Tokenizer tokenizer = Tokenizer_new((String_View){.ptr = program_text, .len = strlen(program_text)});
	
	for (int i = 0; ; ++i) {
		Token *p = tok_next_token(&tokenizer);
		if (!p) {
			break;
		}	

		Token tok = *p;

		printf("%d:  type:`%s`, word: `"FMT_SV"`\n", i, str_EnumToken(tok.type), ARGS_SV(tok.text));
		push_Token_array(&actual, tok);
	}

	for (int i = 0; i < MIN( (int)COUNT_OF(target), actual.size); i++ ){
		if (target[i] != actual.ptr[i].type) {
			fprintf(stderr, "Comparison failed at %d index.  expToken { %s } foundToken { %s }, word = %s\n", 
				i, 
				str_EnumToken(target[i]),
				str_EnumToken(actual.ptr[i].type),
				actual.ptr[i].text.ptr
			);
			break;
		}
	}

	clear_Token_array(tokenizer.tokens);


    return 0;
}