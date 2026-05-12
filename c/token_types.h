
#ifndef MY_TOKEN_TYPES_H

#define MY_TOKEN_TYPES_H


#include <string_view.h>

enum EnumToken {

	T_CUSTOM_WORD,	
	T_SEMICOLON,
	
	T_LIT_STRING ,
	T_LIT_CHAR   ,
	T_LIT_I  ,
	T_LIT_F  ,

	
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

	T_KWD_RETURN,
	T_KWD_TYPE_INT,	
	T_KWD_TYPE_CHAR,	
	T_KWD_TYPE_VOID,	


	T_QUOTE,

    T_COMMA,

    T_L_CURL,
    T_R_CURL,

    
    T_ASTERISC,
};


static const char* str_EnumToken(enum EnumToken t) {
	switch (t) {
	case T_OP_PLUS: 	return "+"; 
	case T_OP_MINUS: 	return "-"; 
	case T_OP_DIVIDE: 	return "/"; 
	case T_OP_PERCENT: 	return "%"; 
	case T_L_BR: 		return "("; 
	case T_R_BR: 		return ")"; 
	case T_L_SQR: 		return "["; 
	case T_R_SQR: 		return "]"; 
	case T_KWD_TYPE_INT: 	return "int";
	case T_KWD_TYPE_CHAR: 	return "char";
	case T_KWD_TYPE_VOID: 	return "void";
	case T_KWD_RETURN: 	return "return";
	case T_ASSIGN: 		return "=";
	case T_CUSTOM_WORD: return "word";
	case T_SEMICOLON: 	return ";";
	case T_LIT_CHAR: 	return "char";
	case T_LIT_STRING:  return "string";
	case T_LIT_I: 		return "num";
	case T_LIT_F: 		return "num_float";
	default: assert(false && "Unknown token: [%d]");
	}
}

typedef struct Tag {
	int pos;
	int row;
	int col;
	int tok_index;
} Tag;


typedef struct Separator {
	String_View text;
	enum EnumToken tok_type;
	Tag tag;
} Separator;


const Separator SEPARATORS[] = {
	{ .text = SV_FROM_CSTR("+"), .tok_type = T_OP_PLUS },
	{ .text = SV_FROM_CSTR("-"), .tok_type = T_OP_MINUS },
	{ .text = SV_FROM_CSTR("*"), .tok_type = T_OP_ASTERISK },
	{ .text = SV_FROM_CSTR("/"), .tok_type = T_OP_DIVIDE },
	{ .text = SV_FROM_CSTR("%"), .tok_type = T_OP_PERCENT },
	{ .text = SV_FROM_CSTR("("), .tok_type = T_L_BR },
	{ .text = SV_FROM_CSTR(")"), .tok_type = T_R_BR },
	{ .text = SV_FROM_CSTR("["), .tok_type = T_L_SQR },
	{ .text = SV_FROM_CSTR("]"), .tok_type = T_R_SQR },
	{ .text = SV_FROM_CSTR("="), .tok_type = T_ASSIGN },
	{ .text = SV_FROM_CSTR(";"), .tok_type = T_SEMICOLON },
	{ .text = SV_FROM_CSTR("\""),.tok_type =  T_QUOTE },
	{ .text = SV_FROM_CSTR("'"), .tok_type = T_QUOTE },
};




#endif