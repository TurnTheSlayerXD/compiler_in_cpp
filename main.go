package main

import (
	"fmt"
)
import . "unicode"
import "strings"


func panic_fmt(format string, args ...any) {
	panic(fmt.Sprintf(format, args))
}

func assert(t bool, msg string) {
	if (t) {
		panic(msg)
	}
}

type EnumToken int
const (
	T_SEMICOLON EnumToken = iota


	T_TYPE_INT EnumToken 	= iota
	T_TYPE_CHAR EnumToken 	= iota
	T_TYPE_VOID EnumToken 	= iota
	
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
	text string
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
	return &Tokenizer{ text: text, tokens: [](Token){}, rune_text: []rune(text), pos: 0, row: 0, col: 0, tok_index: 0}
}



func (t *Tokenizer) trim_left() {
	for t.pos < len([]rune(t.text)) && IsSpace(t.rune_text[t.pos]) {
		t.incr_pos()
	}
}

func (t *Tokenizer) incr_pos() {
	assert(0 <= t.pos && t.pos < len(t.rune_text), "")
	if t.char_at(t.pos) == '\n' {
		t.row += 1
		t.col = 0
	} else {
		t.col += 1
	}
	t.pos += 1
}

func (t *Tokenizer) skip_until(chars string) {
	for t.pos < len([]rune(t.text)) && !strings.Contains(string(t.rune_text[t.pos]),chars) {
		t.incr_pos()
	}
}

func (t *Tokenizer) skip_until_separator() {
	for t.pos < len([]rune(t.text)) && !IsSpace(t.rune_text[t.pos]) {
		t.incr_pos()
	}
}

func (t *Tokenizer) substr(l int, r int) string {
	assert(l <= r, "")
	return string(t.rune_text[l:r])
}


func (t *Tokenizer) eof() bool {
	return t.pos >= len(t.rune_text)
}

func (t *Tokenizer) char_at(i int) rune {
	assert(-1 < i && i < len(t.rune_text), "")
	return t.rune_text[i]
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

		if t.char_at(t.pos) == '#' {
			continue
		}

		// Проверяем единичные символы 
		var ret Token  
		var matched = false
		switch t.char_at(t.pos) {
			case '+': matched = true; ret = Token_new(T_OP_PLUS, tag_start);
			case '-': matched = true; ret = Token_new(T_OP_MINUS, tag_start)
			case '*': matched = true; ret = Token_new(T_OP_ASTERISK, tag_start)
			case '/': matched = true; ret = Token_new(T_OP_DIVIDE, tag_start)
			case '%': matched = true; ret = Token_new(T_OP_PERCENT, tag_start)
			case '(': matched = true; ret = Token_new(T_L_BR, tag_start)
			case ')': matched = true; ret = Token_new(T_R_BR, tag_start)
			case '[': matched = true; ret = Token_new(T_L_SQR, tag_start)
			case ']': matched = true; ret = Token_new(T_R_SQR, tag_start)
			case ';': matched = true; ret = Token_new(T_SEMICOLON, tag_start)
		}
		if matched {
			return true, ret
		}

		// Проверяем ключевые слова
		t.skip_until_separator()
		switch t.substr(tag_start.pos, t.pos) {
			case "int": matched = true; ret = Token_new(T_TYPE_INT, tag_start)
			case "char": matched = true; ret = Token_new(T_TYPE_CHAR, tag_start)
			case "void": matched = true; ret = Token_new(T_TYPE_VOID, tag_start)
			case "return": matched = true; ret = Token_new(T_KWD_RETURN, tag_start)
		}
		if matched {
			return true, ret
		}

		// Проверяем строки и 
	}
}

func Token_new(typeof EnumToken, tag TextTag) Token {
	return Token{ typeof : typeof, tag: tag }
}

func (t *Tokenizer) get_pos() TextTag {
	return TextTag{ pos: t.pos, row: t.row, col: t.col }
}

func (t *Tokenizer) reset_pos(text_pos TextTag) {

}



func main() {

	var programText = "int val = 1;"+"";


	tokenizer := Tokenizer_new(programText)

	
	var _ = tokenizer
}