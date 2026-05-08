#include "tokenizer.c"


int main() {

	const char * program_text = "int val = 1;";
	EnumToken target[] = { T_TYPE_INT, T_CUSTOM_WORD, T_ASSIGN, T_LIT_I, T_SEMICOLON };
	
    Token_array *actual = {0};
    Tokenizer tokenizer = Tokenizer_new(program_text);

	
	for (int i = 0; ; ++i) {
		Token tok = next_tok(&tokenizer);
		if !ok {
			break
		}
		fmt.Printf("%d:  type:`%s`  value:`%+v` \n", i, str_EnumToken(tok.typeof), tok)
		tok_types = append(tok_types, tok.typeof)
		_ = tok
	}

	for i := 0; i < min(len(target_tok_types), len(tok_types)); i++ {
		if target_tok_types[i] != tok_types[i] {
			panic_fmt("Comparison failed at %d index.  expToken { %s } foundToken { %s }", 
				i, 
				str_EnumToken(target_tok_types[i]),
				str_EnumToken(tok_types[i]),
			);
		}
	}
	
	
	assert(slices.Equal(target_tok_types, tok_types), fmt.Sprintf("Comparison Failed, program: `%s`", programText))

	var _ = tokenizer

    return 0;
}