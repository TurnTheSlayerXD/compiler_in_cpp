
#include "../tokenizer.c"

#define TEST_ARG(arr) (struct test_target_arg){ .ptr = (arr), .size = COUNT_OF(arr) }
struct test_target_arg {
    enum EnumToken *ptr;  int size;
};

void test(const char *test_id, const char *program_text, struct test_target_arg target ) {
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

	for (int i = 0; i < MIN(target.size, actual.size); i++ ){
		if (target.ptr[i] != actual.ptr[i].type) {
			fprintf(stderr, "Comparison failed at %d index.  expToken { %s } foundToken { %s }, word = %s\n", 
				i, 
				str_EnumToken(target.ptr[i]),
				str_EnumToken(actual.ptr[i].type),
				actual.ptr[i].text.ptr
			);
			break;
		}
	}

    clear_Token_array(&tokenizer.tokens);

    printf("------------------------------------------------------------------------------------------------------\n");

    printf("SUCCESFUL TEST %s!\n", test_id);

    printf("------------------------------------------------------------------------------------------------------\n");
}


#define P99_PROTECT(...) __VA_ARGS__ 



 

int main() {

    enum EnumToken arg1[] = { T_KWD_TYPE_INT, T_CUSTOM_WORD, T_ASSIGN, T_LIT_I, T_SEMICOLON };
    test("1", "int val = 1;", TEST_ARG(arg1) );


;

    enum EnumToken arg2[] = { 
        T_KWD_TYPE_INT, T_CUSTOM_WORD, T_L_BR, T_KWD_TYPE_INT, T_CUSTOM_WORD, T_COMMA, T_KWD_TYPE_CHAR, T_ASTERISC, T_ASTERISC, T_CUSTOM_WORD, T_R_BR, T_L_CURL, T_R_CURL };
    test("1",
        "#include <stdio.h>\n"
        "\n"
        "int main(int argc, char **argv) {\n"
        "\n"
        "\n"
        "    for (int i = 0; i < argc; i = i + 1) {\n"
        "        printf(\"%s\", argv[i]);\n"
        "    }\n"
        "\n"
        "return 0;\n"
        "}\n", 
        TEST_ARG(arg1) 
    );

}



