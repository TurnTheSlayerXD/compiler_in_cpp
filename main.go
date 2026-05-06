package main

import (
	"fmt"
)
import . "unicode"
import "strconv"
import "strings"
import "slices"

func panic_fmt(format string, args ...any) {
	panic(fmt.Sprintf(format, args))
}

func assert(t bool, msg string) {
	if (!t) {
		panic(msg)
	}
}


type EnumToken int
const (

	T_CUSTOM_WORD EnumToken = iota	

	T_SEMICOLON EnumToken = iota
	
	T_LIT_STRING EnumToken  = iota
	T_LIT_CHAR EnumToken  = iota
	T_LIT_I EnumToken = iota
	T_LIT_F EnumToken = iota

	T_TYPE_INT EnumToken 	= iota
	T_TYPE_CHAR EnumToken 	= iota
	T_TYPE_VOID EnumToken 	= iota

	T_ASSIGN EnumToken = iota

	T_COMP_EQ EnumToken = iota
	T_COMP_LE EnumToken = iota
	T_COMP_LE_OR_EQ EnumToken = iota
	T_COMP_GR EnumToken = iota
	T_COMP_GR_OR_EQ EnumToken = iota

	
	T_OP_PLUS EnumToken 	= iota
	T_OP_MINUS EnumToken 	= iota
	T_OP_ASTERISK EnumToken = iota
	T_OP_DIVIDE EnumToken 	= iota
	T_OP_PERCENT EnumToken 	= iota
	T_L_BR EnumToken 		= iota
	T_R_BR EnumToken 		= iota
	T_L_SQR EnumToken		= iota
	T_R_SQR EnumToken		= iota

	T_KWD_RETURN EnumToken  = iota
)

type Token struct {
	typeof EnumToken
	
	value string
	
	tag TextTag
}

type Separator struct {
	str string
	
	is_space bool
	tok_type EnumToken
}

type Tokenizer struct {
	text string
	
	separators []Separator

	tokens []Token
	
	pos int
	row int
	col int

	tok_index int
} 

type TextTag struct {
	pos int
	row int
	col int
}

func str_EnumToken(t EnumToken) string {
	switch (t) {
	case T_OP_PLUS: 	return "+"; 
	case T_OP_MINUS: 	return "-"; 
	case T_OP_ASTERISK: return "*"; 
	case T_OP_DIVIDE: 	return "/"; 
	case T_OP_PERCENT: 	return "%"; 
	case T_L_BR: 		return "("; 
	case T_R_BR: 		return ")"; 
	case T_L_SQR: 		return "["; 
	case T_R_SQR: 		return "]"; 
	case T_TYPE_INT: 	return "int";
	case T_TYPE_CHAR: 	return "char";
	case T_TYPE_VOID: 	return "void";
	case T_KWD_RETURN: 	return "return";
	case T_ASSIGN: 		return "=";
	case T_CUSTOM_WORD: return "[word]";
	case T_SEMICOLON: return ";";
	case T_LIT_CHAR: return "char";
	case T_LIT_I: return "[num]";
	case T_LIT_F: return "[f_num]";
	default: panic(fmt.Sprintf("Unknown token: [%d]", t));
	}
}


func Tokenizer_new(text string) *Tokenizer {
	var ret = &Tokenizer{ text: text, tokens: [](Token){}, pos: 0, row: 0, col: 0, tok_index: 0 }
	ret.separators = []Separator { 
		Separator{tok_type: T_OP_PLUS, is_space: false, },
		Separator{tok_type: T_OP_MINUS, is_space: false, },
		Separator{tok_type: T_OP_ASTERISK, is_space: false, },
		Separator{tok_type: T_OP_DIVIDE, is_space: false, },
		Separator{tok_type: T_OP_PERCENT, is_space: false, },
		Separator{tok_type: T_L_BR, is_space: false, },
		Separator{tok_type: T_R_BR, is_space: false, },
		Separator{tok_type: T_L_SQR, is_space: false, },
		Separator{tok_type: T_R_SQR, is_space: false, },
		Separator{tok_type: T_TYPE_INT, is_space: false,},
		Separator{tok_type: T_TYPE_CHAR, is_space: false,},
		Separator{tok_type: T_TYPE_VOID, is_space: false,},
		Separator{tok_type: T_KWD_RETURN, is_space: false,},
		Separator{tok_type: T_ASSIGN, is_space: false,},
		Separator{tok_type: T_SEMICOLON, is_space: false,},
	}
	for i := range len(ret.separators) {
		ret.separators[i].str = str_EnumToken(ret.separators[i].tok_type)
	}
	return ret
}


func (t *Tokenizer) trim_left() {
	for t.pos < len(t.text) && IsSpace(rune(t.text[t.pos])) {
		t.incr_pos()
	}
}

func (t *Tokenizer) incr_pos() {
	assert(0 <= t.pos && t.pos < len(t.text), "")
	if t.text[t.pos] == '\n' {
		t.row += 1
		t.col = 0
	} else {
		t.col += 1
	}
	t.pos += 1
}

func (t *Tokenizer) skip_until(chars string) (ok bool, value string) {
	var init = t.pos
	for t.pos < len(t.text) {
		if strings.ContainsRune(chars, rune(t.text[t.pos])) {
			return true, t.text[init : t.pos]
		}
		
		t.incr_pos()
	}

	return false, ""
}

func (t *Tokenizer) skip_until_char(char byte) (ok bool, value string) {
	var init = t.pos
	for t.pos < len(t.text) {
		if t.char_cur() == char {
			return true, t.text[init : t.pos]
		}
		
		t.incr_pos()
	}

	return false, ""
}


func (t *Tokenizer) is_cur_separator() *Separator {
	if IsSpace(rune(t.char_cur())) {
		return &Separator{ is_space: true }
	}
	
	for i := range t.separators {
		if strings.HasPrefix(t.text[t.pos:], t.separators[i].str) {
			return &t.separators[i]
		} 
	} 

	return nil
}

func (t *Tokenizer) skip_until_separator() (string, *Separator) {
	var init = t.pos
	var sep *Separator
	for t.pos < len(t.text) {
		if sep = t.is_cur_separator(); sep != nil {
			return t.text[init : t.pos], sep
		} 
		t.incr_pos()
	}

	return t.text[init : t.pos], nil
}


func (t *Tokenizer) eof() bool {
	return t.pos >= len(t.text)
}


func (t *Tokenizer) char_cur() byte {
	assert( -1 < t.pos && t.pos < len(t.text), "")
	return t.text[t.pos]
}


func (t *Tokenizer) next_tok() (ok bool, tok Token) {
	
	if t.tok_index < len(t.tokens) {
		t.tok_index += 1
		return true, t.tokens[t.tok_index - 1]
	}


	for {
		t.trim_left()
		if (t.eof()) {
			return false, Token{}
		}

		var tag_start = t.get_pos()

		if t.char_cur() == '#' {
			t.skip_until("\n")
			continue
		}

		// Проверяем единичные символы 
		
		// Проверяем кавычки 
		if (t.char_cur() == '"' || t.char_cur() == '\'') {
			var quote = t.char_cur()
			
			t.incr_pos()
			var found, value = t.skip_until_char(quote)
			if !found {
				panic_fmt("Unmatched quote at %s", tag_start)
			}

			t.incr_pos()

			if quote == '"' {
				return true, t.Token_new_with_value(T_LIT_STRING, tag_start, value)
			} else if quote == '\'' {
				return true, t.Token_new_with_value(T_LIT_CHAR, tag_start, value)
			} else {
				panic("UNEXPECTED")
			}
		}

		// Доходим до сепаратораы
		var word, sep = t.skip_until_separator()
		
		var is_ret = false

		// Мы не можем тут сразу сделать return, потому что нам нужно заппендеить токен сепаратор, иначе он потеряется
		if len(word) > 0 {
			is_ret = true
			// Проверяем если целое число
			//TODO RADIX NOT 10 Only
			if _, err := strconv.ParseInt(word, 10, 32); err == nil {
				t.Token_new_with_value(T_LIT_I, tag_start, word)
			} else if _, err := strconv.ParseFloat(word, 32); err == nil {
				t.Token_new_with_value(T_LIT_F, tag_start, word)
			} else {
				t.Token_new_with_value(T_CUSTOM_WORD, tag_start, word)
			}
		}
		
		if sep != nil && !sep.is_space {
			t.Token_new(sep.tok_type, t.get_pos())
	
			if is_ret {
				t.tok_index = len(t.tokens) - 1
				// Смещаем на len-1, потому что мы токен-сепаратор возвращаем НЕ СРАЗУ
			} else {
				is_ret = true
			}

			// Теперь нам нужно сместить курсор на длину сепаратора
			for i := 0; i < len(sep.str); i++ {
				t.incr_pos()
			}
		}

		if !is_ret && !t.eof() {
			panic("UNREACHABLE")
		}

		if is_ret {
			return true, t.tokens[t.tok_index - 1] 
		}

		return false, Token{}
	}
}


func (t *Tokenizer) Token_new(typeof EnumToken, tag TextTag) Token {
	var tok = Token{ typeof : typeof, tag: tag }
	t.tokens = append(t.tokens, tok)
	t.tok_index = len(t.tokens)
	return tok
}

func (t *Tokenizer) Token_new_with_value(typeof EnumToken, tag TextTag, value string) Token {
	var tok = Token{ typeof : typeof, tag: tag, value: value}
	t.tokens = append(t.tokens, tok)
	t.tok_index = len(t.tokens)
	return tok
}


func (t *Tokenizer) get_pos() TextTag {
	return TextTag{ pos: t.pos, row: t.row, col: t.col }
}

func (t *Tokenizer) reset_pos(text_pos TextTag) {
}


func main() {

	var programText = "int val = 1;"+"";
	var target_tok_types = []EnumToken{ T_TYPE_INT, T_CUSTOM_WORD, T_ASSIGN, T_LIT_I, T_SEMICOLON }
	var tok_types = []EnumToken{}

	tokenizer := Tokenizer_new(programText)

	i := 0
	for {
		i++
		var ok, tok = tokenizer.next_tok()
		if !ok {
			break
		}
		fmt.Printf("%d:  type:`%s`  value:`%s` \n", i, str_EnumToken(tok.typeof), tok.value)
		tok_types = append(tok_types, tok.typeof)
		_ = tok
	}
	
	assert(slices.Equal(target_tok_types, tok_types), fmt.Sprintf("Comparison Failed, program: `%s`", programText))

	var _ = tokenizer
}