
#include <stdio.h>

enum EnumToken {

	T_CUSTOM_WORD,	
	T_SEMICOLON,
	
	T_LIT_STRING ,
	T_LIT_CHAR   ,
	T_LIT_I  ,
	T_LIT_F  ,

	T_TYPE_INT  ,	
	T_TYPE_CHAR  ,	
	T_TYPE_VOID  ,	

	T_ASSIGN  ,

	T_COMP_EQ  ,
	T_COMP_LE  ,
	T_COMP_LE_OR_EQ,  
	T_COMP_GR  ,
	T_COMP_GR_OR_EQ,  

	
	T_OP_PLUS  	,
	T_OP_MINUS  	,
	T_OP_ASTERISK,  
	T_OP_DIVIDE  ,	
	T_OP_PERCENT,
	T_L_BR,  		
	T_R_BR,  		
	T_L_SQR, 		
	T_R_SQR, 		

	T_KWD_RETURN
};

typedef struct TextTag {
	int pos;
	int row;
	int col;
	int tok_index;
} TextTag;


typedef struct Separator {

} Separator;

typedef struct Token {
    enum EnumToken tp;
	const char *value;
	struct TextTag tag;
} Token;

typedef struct Tokenizer {
    const char *text;
	struct Separator separators[10];
	struct Token *tokens;
	int pos; 
	int row; 
	int col; 
	int cur_tok_index; 
} Tokenizer;


struct Token_array {
    struct Token *ptr;
    size_t cap;
    size_t size;
};

void push_Token_array(Token_array *p, Token t) {
    if (p.cap >= p.size) {
        p.ptr = realloc(sizeof(Token) * (p.cap * 2 + 1));
        p.cap = p.cap * 2 + 1;
    }
    
    p.ptr[p.size] = t;
    p.size += 1;
}


void clear_Token_array(Token_array *p) {
    free(p.ptr);
    p.ptr = NULL:
    p.size = 0;
    p.cap = 0;
}

Tokenizer Tokenizer_new(const char *text) {
    return (struct Tokenizer){ 
        .text = text, 
        .tokens = {}, 
        .pos = 0,
        .row = 0, 
        .col = 0, 
        .cur_tok_index = 0 
    };
}
