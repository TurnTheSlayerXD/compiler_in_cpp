#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include <strdup.h>
#include <string_view.h>

#include "token_types.h"


#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))




typedef struct Token {
    enum EnumToken type;
	String_View text;
	Tag tag;
} Token;


typedef struct Token_array {
    Token *ptr;
    int cap;
    int size;
} Token_array;

typedef struct Tokenizer {
    String_View text;
	Token_array tokens;
	Tag cur; 
} Tokenizer;


void push_Token_array(Token_array *p, Token t) {
    if (p->cap >= p->size) {
        p->ptr = realloc(p->ptr, sizeof(Token) * (p->cap * 2 + 1));
        p->cap = p->cap * 2 + 1;
    }
    p->ptr[p->size] = t;
    p->size += 1;
}


void clear_Token_array(Token_array *p) {
    free(p->ptr);
    p->ptr = NULL;
    p->size = 0;
    p->cap = 0;
}


Tokenizer Tokenizer_new(String_View text) {
    return (Tokenizer){ 
        .text = text, 
        .tokens = {0}, 
        .cur = (Tag){ .pos = 0, .row = 0, .col = 0, .tok_index = 0 },
    };
}

bool tok_eof(Tokenizer *this) {
	return this->cur.pos >= this->text.len;
}

void tok_iter_next(Tokenizer *this) {
	if (tok_eof(this)) {
		assert(false && "tok_iter_next tok_eof");
	}
	if (this->text.ptr[this->cur.pos] == '\n') {
		this->cur.row += 1;
		this->cur.col = 0;
	}
	else {
		this->cur.col += 1;
	}
	this->cur.pos += 1;
}

int try_int_conversion(String_View str, bool *err) {
	if (str.len < 0) {
		assert(false && "try_float_conversion(String_View str, bool *err)");
	}
	char *heap_str = strdup_n(str.ptr, str.len);
	char *end_ptr;
	int conv = strtol(heap_str, &end_ptr, 10);
	*err = end_ptr == heap_str;
	free(heap_str);
	return conv;
}

float try_float_conversion(String_View str, bool *err) {
	if (str.len < 0) {
		assert(false && "try_float_conversion(String_View str, bool *err)");
	}
	char *heap_str = strdup_n(str.ptr, str.len);
	char *end_ptr;
	float conv = strtof(heap_str, &end_ptr);
	*err = end_ptr == heap_str;
	free(heap_str);
	return conv;
}

void tok_skip_until_separator(Tokenizer *this, String_View *ret_word, Separator *ret_sep) {
	size_t c = 0;

	while (!tok_eof(this)) {
		if (isspace(this->text.ptr[this->cur.pos])) {
			*ret_word = substr_sv(this->text, this->cur.pos - c, this->cur.pos);
			break;
		}

		int sep_index = -1;
		for (int i = 0; i < (int)(COUNT_OF(SEPARATORS)); ++i) {
			if (startswith_sv(left_substr_sv(this->text, this->cur.pos), SEPARATORS[i].text)) {
				sep_index = i;
			}
		}

		if (sep_index != -1) {
			*ret_sep = SEPARATORS[sep_index];
			ret_sep->tag = this->cur;
			*ret_word = substr_sv(this->text, this->cur.pos - c, this->cur.pos);
			break;
		}

		tok_iter_next(this);
		c += 1;
	}
	
}


char tok_get_cur_char(Tokenizer *this) {
	if (tok_eof(this)) {
		assert(false && "tok_get_cur_char()");
	}
	return this->text.ptr[this->cur.pos];
}

void tok_add_new(Tokenizer *this, enum EnumToken tp, Tag tag) {
	push_Token_array(&this->tokens, (Token){ .tag = tag, .type = tp, .text = {0} });
	this->cur.tok_index = this->tokens.size; 
}
void tok_add_new_with_text(Tokenizer *this, enum EnumToken tp, Tag tag, String_View text) {
	push_Token_array(&this->tokens, (Token){ .tag = tag, .type = tp, .text = text });
	this->cur.tok_index = this->tokens.size; 
}

void tok_trim_left(Tokenizer *this) {
	while (!tok_eof(this) && isspace(tok_get_cur_char(this))) {
		tok_iter_next(this);
	}
}

enum EnumToken try_recognize_keyword(String_View str, bool *is_keyword) {
	static struct { String_View str; enum EnumToken t; } kwds[] = {  
		{ .str = SV_FROM_CSTR("int"), 	 .t = T_KWD_TYPE_INT 	},
		{ .str = SV_FROM_CSTR("char"),   .t = T_KWD_TYPE_CHAR 	},
		{ .str = SV_FROM_CSTR("void"),   .t = T_KWD_TYPE_VOID 	},
		{ .str = SV_FROM_CSTR("return"), .t = T_KWD_RETURN 		},
	};
	for (int i = 0; i < (int)COUNT_OF(kwds); ++i) {
		if (comp_eq_sv(str, kwds[i].str)) {
			*is_keyword = true;
			return kwds[i].t;
		}
	}
	*is_keyword = false;
	return 0;
}


#define FMT_TAG "Tag pos:%d,row:%d,col:%d "
#define FMT_ARGS_TAG(p) (p.pos),(p.row),(p.col)

 Token* tok_next_token(Tokenizer* this)  {
	if (this->cur.tok_index < this->tokens.size) {
		this->cur.tok_index += 1;
		return &this->tokens.ptr[this->cur.tok_index - 1];
	}

	for (;;) {
		tok_trim_left(this);

		if (tok_eof(this)) {
			return NULL;
		}

		Tag tag_start = this->cur;

		if (tok_get_cur_char(this) == '#') {
			while (!tok_eof(this) && tok_get_cur_char(this) != '\n') {
				tok_iter_next(this);
			}
			continue;
		}
		
		// Доходим до сепаратора
		String_View word = {0};
		Separator sep = {0};
		tok_skip_until_separator(this, &word, &sep);
		
		bool is_word_before_separator = false;
		if (word.len > 0) {
			is_word_before_separator = true;
			enum EnumToken tok_type = T_CUSTOM_WORD; 
			if (tok_type == T_CUSTOM_WORD) {
				bool err = false;
				try_int_conversion(word, &err);
				if (!err) {
					tok_type = T_LIT_I;
				}
			}
			if (tok_type == T_CUSTOM_WORD) {
				bool err = false;
				try_float_conversion(word, &err);
				if (!err) {
					tok_type = T_LIT_F;
				}
			}
			if (tok_type == T_CUSTOM_WORD) {
				bool is_keyword = false;
				enum EnumToken kwd_type = try_recognize_keyword(word, &is_keyword);
				if (is_keyword) {
					tok_type = kwd_type;
				}
			}

			tok_add_new_with_text(this, tok_type, tag_start, word);
		}

		if (sep.text.len > 0) {

			bool is_string_quote = sep.tok_type == T_LIT_STRING;
			bool is_char_quote = sep.tok_type == T_LIT_CHAR;
			char quote;
			if (is_char_quote) {
				quote = '\'';
			} else if (is_string_quote) {
				quote = '"';
			}
		
			if (is_string_quote || is_char_quote) {
				tok_iter_next(this);
				String_View text_inside_quotes = {0};
				text_inside_quotes.ptr = this->text.ptr + sep.tag.pos + 1;
				int c = 0;
				while (!tok_eof(this) && tok_get_cur_char(this) != quote && tok_get_cur_char(this) != '\n') {
					tok_iter_next(this);
					c += 1;
				}

				if (tok_eof(this) || tok_get_cur_char(this) != quote) {
					fprintf(stderr, ""FMT_TAG"", FMT_ARGS_TAG(tag_start));
					return NULL;
				}
				text_inside_quotes.len = c;
				tok_iter_next(this);
				tok_add_new_with_text(this, sep.tok_type, sep.tag, text_inside_quotes);	
			}
			else {
				tok_add_new(this, sep.tok_type, sep.tag);
				// Теперь нам нужно сместить курсор на длину сепаратора, который не ВНУТРИ КАВЫЧЕК
				for (int i = 0; i < sep.text.len; ++i) {
					tok_iter_next(this);
				}
			}

			if (is_word_before_separator) {
				// Смещаем до len-1, потому что мы нашли слово и токен-сепаратор возвращаем НЕ СРАЗУ
				this->cur.tok_index = this->tokens.size - 1;
			}
			
		}

		if (word.len > 0 || sep.text.len > 0) {
			return &this->tokens.ptr[this->cur.tok_index - 1];
		}

		return NULL;
	}
}


