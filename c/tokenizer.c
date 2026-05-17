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


#define FMT_TAG "Tag pos:%d,row:%d,col:%d "
#define FMT_ARGS_TAG(p) (p.pos),(p.row),(p.col)

typedef struct Tag {
	int pos;
	int row;
	int col;
	int tok_index;
} Tag;

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

	int pos;
	int row;
	int col;
	int tok_index;
} Tokenizer;


void push_Token_array(Token_array *p, Token t) {
    if (p->cap <= p->size) {
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
	init_tokens();
    return (Tokenizer){ 
        .text = text, 
        .tokens = {0}, 

		.pos = 0,
		.row = 0,
		.col = 0,
        .tok_index = 0,
    };
}

bool tok_eof(Tokenizer *this) {
	return this->pos >= this->text.len;
}

void tok_iter_next(Tokenizer *this) {
	if (tok_eof(this)) {
		assert(false && "tok_iter_next tok_eof");
	}
	if (this->text.ptr[this->pos] == '\n') {
		this->row += 1;
		this->col = 0;
	}
	else {
		this->col += 1;
	}
	this->pos += 1;
}

static char heap_str[100] = {0};

void prep_heap_str(String_View str) {
	int len = MIN(str.len, COUNT_OF(heap_str));
	if (str.len < COUNT_OF(heap_str)) {
		heap_str[str.len] = '\0';
		memcpy(heap_str, str.ptr, len);
	}
	else {
		heap_str[COUNT_OF(heap_str) - 1] = '\0';
		memcpy(heap_str, str.ptr, COUNT_OF(heap_str) - 1);
	}
}

int try_int_conversion(String_View str, bool *err) {
	if (str.len < 0) {
		assert(false && "try_float_conversion(String_View str, bool *err)");
	}
	prep_heap_str(str);
	char *end_ptr;
	int conv = strtol(heap_str, &end_ptr, 10);
	*err = end_ptr == heap_str;
	return conv;
}

float try_float_conversion(String_View str, bool *err) {
	if (str.len < 0) {
		assert(false && "try_float_conversion(String_View str, bool *err)");
	}
	prep_heap_str(str);
	char *end_ptr;
	float conv = strtof(heap_str, &end_ptr);
	*err = end_ptr == heap_str;
	return conv;
}

bool tok_skip_until_stop_token(Tokenizer *this, String_View *ret_word, enum EnumToken *stop_tok_type) {
	size_t c = 0;

	while (!tok_eof(this)) {
		if (isspace(this->text.ptr[this->pos])) {
			*ret_word = substr_sv(this->text, this->pos - c, this->pos);
			return false;
		}

		bool is_stop_token = startswith_stop_token(left_substr_sv(this->text, this->pos), stop_tok_type);
		if (is_stop_token) {
			*ret_word = substr_sv(this->text, this->pos - c, this->pos);
			return true;
		}

		tok_iter_next(this);
		c += 1;
	}
	
	*ret_word = substr_sv(this->text, this->pos - c, this->pos);
	return false;
}


char tok_get_cur_char(Tokenizer *this) {
	if (tok_eof(this)) {
		assert(false && "tok_get_cur_char()");
	}
	return this->text.ptr[this->pos];
}

void tok_add_new(Tokenizer *this, enum EnumToken tp, Tag tag) {
	push_Token_array(&this->tokens, (Token){ .tag = tag, .type = tp, .text = {0} });
	this->tok_index = this->tokens.size; 
}
void tok_add_new_with_text(Tokenizer *this, enum EnumToken tp, Tag tag, String_View text) {
	push_Token_array(&this->tokens, (Token){ .tag = tag, .type = tp, .text = text });
	this->tok_index = this->tokens.size; 
}

void tok_trim_left(Tokenizer *this) {
	while (!tok_eof(this) && isspace(tok_get_cur_char(this))) {
		tok_iter_next(this);
	}
}



Tag tok_get_cur_pos(Tokenizer *this) {
	return (Tag){ .pos = this->pos, .row = this->row, .col = this->col, .tok_index = this->tok_index };
} 

void tok_reset(Tokenizer *this, Tag tag) {
	if (tag.tok_index > this->tokens.size || tag.tok_index < 0) {
		assert(false && "Invalid .tok_index");
	}
	this->tok_index = tag.tok_index;
}


 Token* tok_next_token(Tokenizer* this)  {
	if (this->tok_index < this->tokens.size) {
		this->tok_index += 1;
		return &this->tokens.ptr[this->tok_index - 1];
	}

	for (;;) {
		tok_trim_left(this);

		if (tok_eof(this)) {
			return NULL;
		}

		Tag tag_start = tok_get_cur_pos(this);

		if (tok_get_cur_char(this) == '#') {
			while (!tok_eof(this) && tok_get_cur_char(this) != '\n') {
				tok_iter_next(this);
			}
			continue;
		}
		
		// Доходим до сепаратора
		String_View word = {0};
	
		enum EnumToken stop_tok_type;
		bool is_stop_token = tok_skip_until_stop_token(this, &word, &stop_tok_type);
		bool is_word_before_stop = word.len > 0;

		if (is_word_before_stop) {
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

		if (is_stop_token) {
			Tag stop_tok_tag = tok_get_cur_pos(this);

			bool is_char_quote = stop_tok_type == T_SINGLE_QUOTE;
			bool is_string_quote = stop_tok_type == T_MULTI_QUOTE;
			char quote;
			if (is_char_quote) {
				quote = '\'';
				stop_tok_type = T_LIT_CHAR; 
			} else if (is_string_quote) {
				quote = '"';
				stop_tok_type = T_LIT_STRING; 
			}
			if (is_string_quote || is_char_quote) {
				tok_iter_next(this);
				int c = 0;
				while (!tok_eof(this)) {
					if(tok_get_cur_char(this) == '\n') {
						fprintf(stderr, ""FMT_TAG"", FMT_ARGS_TAG(tag_start));
						return NULL;
					}
					else if (tok_get_cur_char(this) == '\\') {
						tok_iter_next(this);
					}
					else if (tok_get_cur_char(this) == quote) {
						break;
					}
					tok_iter_next(this);
					c += 1;
				}
				if (tok_eof(this) || tok_get_cur_char(this) != quote) {
					fprintf(stderr, ""FMT_TAG"", FMT_ARGS_TAG(tag_start));
					return NULL;
				}
				String_View text_inside_quotes = { .ptr = this->text.ptr + this->pos - c, .len = c };
				tok_iter_next(this);
				tok_add_new_with_text(this, stop_tok_type, stop_tok_tag, text_inside_quotes);	
			}
			else {
				tok_add_new(this, stop_tok_type, stop_tok_tag);
				// Теперь нам нужно сместить курсор на длину сепаратора
				int times_to_skip = len_of_stop_token(stop_tok_type);
				while (times_to_skip-- > 0) {
					tok_iter_next(this);
				}
			}

			if (is_word_before_stop) {
				// Смещаем до len-1, потому что мы нашли слово и токен-сепаратор вернем потом
				this->tok_index = this->tokens.size - 1;
			}
			
		}

		if (is_word_before_stop || is_stop_token) {
			return &this->tokens.ptr[this->tok_index - 1];
		}

		return NULL;
	}
}


