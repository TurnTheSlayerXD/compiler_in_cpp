package main

import (
	"fmt"
)
import . "unicode"
import "strconv"

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
	T_SEMICOLON EnumToken = iota
	
	T_LIT_STRING EnumToken  = iota
	T_LIT_CHAR EnumToken  = iota
	T_LIT_INT EnumToken = iota
	T_LIT_FLOAT EnumToken = iota

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

type Tokenizer struct {
	_text string
	rune_text []rune 

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


func Tokenizer_new(text string) *Tokenizer {
	return &Tokenizer{ _text: text, tokens: [](Token){}, rune_text: []rune(text), pos: 0, row: 0, col: 0, tok_index: 0}
}



func (t *Tokenizer) trim_left() {
	for t.pos < len(t.rune_text) && IsSpace(t.rune_text[t.pos]) {
		t.incr_pos()
	}
}

func (t *Tokenizer) incr_pos() {
	assert(0 <= t.pos && t.pos < len(t.rune_text), "")
	if t.rune_text[t.pos] == '\n' {
		t.row += 1
		t.col = 0
	} else {
		t.col += 1
	}
	t.pos += 1
}

func (t *Tokenizer) skip_until(chars ...rune) (ok bool, value string) {
	var word_chars = []rune{} 
	for t.pos < len(t.rune_text) {
		var found = false
		for _, c := range chars  {
			if c == t.rune_text[t.pos] {
				found = true
				break
			}
			word_chars = append(word_chars, t.rune_text[t.pos])
		}
		if (found) {
			return true, string(word_chars)
		}
		t.incr_pos()
	}

	return false, ""
}

func (t *Tokenizer) skip_until_separator() string {
	var chars = []rune{}
	for t.pos < len(t.rune_text) && t.rune_text[t.pos] != ';' && !IsSpace(t.rune_text[t.pos]) {
		chars = append(chars, t.rune_text[t.pos])
		t.incr_pos()
	}

	return string(chars)
}


func (t *Tokenizer) eof() bool {
	return t.pos >= len(t.rune_text)
}


func (t *Tokenizer) char_cur() rune {
	assert( -1 < t.pos && t.pos < len(t.rune_text), "")
	return t.rune_text[t.pos]
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
			t.skip_until_separator()
			continue
		}

		// Проверяем единичные символы 
		
		// Проверяем кавычки 
		if (t.char_cur() == '"' || t.char_cur() == '\'') {
			var quote = t.char_cur()
			
			t.incr_pos()
			var found, value = t.skip_until(t.char_cur())
			if !found {
				panic_fmt("Unmatched quote at %s", tag_start)
			}

			t.incr_pos()

			if quote == '"' {
				return true, t.Token_new_with_value(T_STRING, tag_start, value)
			} else if quote == '\'' {
				return true, t.Token_new_with_value(T_CHAR, tag_start, value)
			} else {
				panic("UNEXPECTED")
			}
		}

		//Проверяем отдельно semicolon, потому что это разделитель между выражениями
		if t.char_cur() == ';' {
			t.incr_pos()
			return true, t.Token_new(T_SEMICOLON, tag_start)
		}

		// Проверяем все, что отдельное выражение
		var ret Token  
		var word = t.skip_until_separator()
		switch word {
			case "+": return true, t.Token_new(T_OP_PLUS, tag_start)
			case "-": return true, t.Token_new(T_OP_MINUS, tag_start)
			case "*": return true, t.Token_new(T_OP_ASTERISK, tag_start)
			case "/": return true, t.Token_new(T_OP_DIVIDE, tag_start)
			case "%": return true, t.Token_new(T_OP_PERCENT, tag_start)
			case "(": return true, t.Token_new(T_L_BR, tag_start)
			case ")": return true, t.Token_new(T_R_BR, tag_start)
			case "[": return true, t.Token_new(T_L_SQR, tag_start)
			case "]": return true, t.Token_new(T_R_SQR, tag_start)
			case "int": 	 return true, t.Token_new(T_TYPE_INT, tag_start)
			case "char": 	 return true, t.Token_new(T_TYPE_CHAR, tag_start)
			case "void": 	 return true, t.Token_new(T_TYPE_VOID, tag_start)
			case "return":   return true, t.Token_new(T_KWD_RETURN, tag_start)
		}

		// Проверяем если целое число
		//TODO RADIX NOT 10 Only
		if _, err := strconv.ParseInt(word, 10, 32); err != nil {
			ret = t.Token_new_with_value(T_INT, tag_start, word)
			return true, ret
		}

		if _, err := strconv.ParseFloat(word, 32); err != nil {
			ret = t.Token_new_with_value(T_FLOAT, tag_start, word)
			return true, ret
		}
		
		panic_fmt("UNKNOWN word = {%s}", word)
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

	tokenizer := Tokenizer_new(programText)


	for {
		var ok, tok = tokenizer.next_tok()
		if !ok {
			break
		}

		fmt.Printf("%+v\n", tok)
		_ = tok
	}

	var _ = tokenizer
}