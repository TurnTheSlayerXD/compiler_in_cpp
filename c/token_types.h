
#ifndef MY_TOKEN_TYPES_H

#define MY_TOKEN_TYPES_H

#define TOKENS_LIT \
	X(T_LIT_STRING, "lit_string")\
	X(T_LIT_CHAR, "lit_char")\
	X(T_LIT_I, "lit_int")\
	X(T_LIT_F, "lit_float") 

#define TOKENS_OPERATIONS \
	X(T_OP_PLUS,"+") \
	X(T_OP_MINUS,"-") \
	X(T_OP_DIVIDE,"/") \
	X(T_OP_PERCENT,"%") 

#define TOKENS_CMP \
	X(T_CMP_EQ, "==") \
	X(T_CMP_LE, "<") \
	X(T_CMP_LE_OR_EQ, "<=") \
	X(T_CMP_GR, ">") \
	X(T_CMP_GR_OR_EQ, ">=")

#define TOKENS_BRACES \
	X(T_L_BR,"(")\
	X(T_R_BR,")")\
	X(T_L_SQR,"[")\
	X(T_R_SQR,"]")\
	X(T_L_CURL,"{")\
    X(T_R_CURL,"}")

#define TOKENS_KWDS \
	X(T_KWD_RETURN, "return")\
	X(T_KWD_TYPE_INT, "int")\
	X(T_KWD_TYPE_CHAR, "char")\
	X(T_KWD_TYPE_VOID, "void")\
	X(T_KWD_FOR, "for")
	
#define TOKENS_MAIN \
	X(T_CUSTOM_WORD, "word") \
	X(T_SEMICOLON, ";") \
	X(T_ASSIGN, "=") \
    X(T_COMMA, ",") \
    X(T_STAR, "*")

#define TOKENS_QUOTES \
	X(T_SINGLE_QUOTE, "'") \
	X(T_MULTI_QUOTE, "\"")

#define ALL_TOKENS \
	TOKENS_LIT \
	TOKENS_OPERATIONS \
	TOKENS_CMP \
	TOKENS_BRACES \
	TOKENS_KWDS \
	TOKENS_MAIN \
	TOKENS_QUOTES
	

#define STOP_TOKENS \
	X(T_SEMICOLON) \
	X(T_ASSIGN) \
	X(T_COMMA) \
	X(T_STAR) \
	X(T_L_BR) \
	X(T_R_BR) \
	X(T_L_SQR) \
	X(T_R_SQR) \
	X(T_L_CURL) \
	X(T_R_CURL) \
	X(T_CMP_EQ) \
	X(T_CMP_LE) \
	X(T_CMP_LE_OR_EQ) \
	X(T_CMP_GR) \
	X(T_CMP_GR_OR_EQ) \
	X(T_OP_PLUS) \
	X(T_OP_MINUS) \
	X(T_OP_DIVIDE) \
	X(T_OP_PERCENT) \
	X(T_SINGLE_QUOTE) \
	X(T_MULTI_QUOTE)

enum EnumToken {
	#define X(name, value) name,
		ALL_TOKENS
	#undef X

};


static const char* str_EnumToken(enum EnumToken t) {
	switch (t) {
	
	#define X(name, str) case name: return str;
		ALL_TOKENS
	#undef X

	default: assert(false && "Unknown token: [%d]");
	}
}


struct __predefined {
	 String_View text; enum EnumToken tok_type;
};

struct __predefined SEPARATORS[] = {
	#define X(name) { .tok_type = name } ,
		STOP_TOKENS
	#undef X
};

struct __predefined KEYWORDS[] = {
	#define X(name, value) { .tok_type = name } ,
		TOKENS_KWDS
	#undef X
};

bool __initialized = false;

int comp_callback(const void* lhs, const void* rhs) {
	return ((struct __predefined*)rhs)->text.len - ((struct __predefined*)lhs)->text.len;
}

void init_tokens(void) {
	if (__initialized) {
		return;
	}
	__initialized = true;
	
	for (int i = 0; i < COUNT_OF(SEPARATORS); ++i) {
		const char *str = str_EnumToken(SEPARATORS[i].tok_type);
		SEPARATORS[i].text = (String_View){ .ptr =  str, .len = strlen(str)};
	}
	qsort(SEPARATORS, COUNT_OF(SEPARATORS), sizeof(SEPARATORS[0]), comp_callback);

	for (int i = 0; i < COUNT_OF(KEYWORDS); ++i) {
		const char *str = str_EnumToken(KEYWORDS[i].tok_type);
		KEYWORDS[i].text = (String_View){ .ptr =  str, .len = strlen(str)};
	}
	qsort(KEYWORDS, COUNT_OF(KEYWORDS), sizeof(KEYWORDS[0]), comp_callback);
}

#include <string_view.h>

bool startswith_stop_token(String_View view, enum EnumToken *ret) {
	for (int i = 0; i < COUNT_OF(SEPARATORS); ++i) {
		if (startswith_sv(view, SEPARATORS[i].text)) {
			*ret = SEPARATORS[i].tok_type;
			return true; 		
		}
	}

	return false;
}

int len_of_stop_token(enum EnumToken t) {
		
	for (int i = 0; i < COUNT_OF(SEPARATORS); ++i) {
		if (SEPARATORS[i].tok_type == t) {
			return SEPARATORS[i].text.len;
		}
	}

	assert(false && "false calll to len_of_stop_token");

	return 0;
}

enum EnumToken try_recognize_keyword(String_View str, bool *is_keyword) {
	for (int i = 0; i < COUNT_OF(KEYWORDS); ++i) {
		if (comp_eq_sv(str, KEYWORDS[i].text)) {
			*is_keyword = true;
			return KEYWORDS[i].tok_type;
		}
	}
	*is_keyword = false;
	return 0;
}


#endif
