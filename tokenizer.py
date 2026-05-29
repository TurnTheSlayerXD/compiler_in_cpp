from enum import auto, Flag

import copy

from dataclasses import dataclass

import re

@dataclass
class TextPos:
    row: int
    col: int
    i: int

    def __init__(self, row, col, i):
        self.row = row
        self.col = col
        self.i = i


class TokenType(Flag):
    T_WORD = auto()

    T_FLOAT = auto()
    T_INT = auto()

    T_STR = auto()
    T_CHAR = auto()

    T_PLUS = auto()
    T_MINUS = auto()

    T_L_BR = auto()
    T_R_BR = auto()

    T_MUL = auto()
    T_DIV = auto()

    T_ASSIGN = auto()

    T_RETURN = auto(),
    T_BREAK = auto(),
    T_CONTINUE = auto(),
    T_IF = auto(),
    T_ELSE = auto(),


class Token:
    type: TokenType
    text_pos: TextPos
    text: str | None
    index: int

    def __init__(self, type: TokenType, index: int, text_pos: TextPos, text: str | None = None):
        self.type = type
        self.index = index
        self.text_pos = text_pos
        self.text = text

    def __str__(self):
        return ''

class Tokenizer:
    text: str
    tokens: list[Token]
    tok_index: int
    cur: TextPos
    char: str
    cur_stop: TokenType | None
    cur_kwd: TokenType | None

    def __init__(self, text: str):
        self.text = text
        self.tokens = []
        self.tok_index = 0
        self.cur = TextPos(0, 0, 0)
        self._iter()
        self.cur_stop = None
        self.cur_kwd = None

        self.STOP_TOKENS = [
            ("=", TokenType.T_ASSIGN,),
            ("+", TokenType.T_PLUS,),
            ("-", TokenType.T_MINUS,),
            ("*", TokenType.T_MUL,),
            ("/", TokenType.T_DIV,),
            (";", TokenType.T_DIV,),
            ("(", TokenType.T_L_BR,),
            (")", TokenType.T_R_BR,),
            ("'", TokenType.T_CHAR,),
            ("\"", TokenType.T_STR,),
        ]
        self.KWD_TOKENS = [
            ("return", TokenType.T_RETURN),
            ("break", TokenType.T_BREAK),
            ("continue", TokenType.T_CONTINUE),
            ("if", TokenType.T_IF),
            ("else", TokenType.T_ELSE),
        ]

        self.STOP_TOKENS.sort(reverse=True)
        self.KWD_TOKENS.sort(reverse=True)

    def get_cur_pos(self) -> TextPos:
        return self.tok_index

    def reset_pos(self, index: int):
        self.tok_index = index

    def eof(self) -> bool:
        self._trim_left()
        return self._eof()

    def add_token(self, token):
        self.tokens.append(token)
        self.tok_index = len(self.tokens)

    def next_tok(self) -> Token:
        if self.tok_index < len(self.tokens):
            self.tok_index += 1
            return self.tokens[self.tok_index - 1]

        while True:
            self._trim_left()
            if self._eof():
                raise Exception("EOF Tokens: Invalid access")

            if self.char == '#':
                while self.char != "\n" and self._iter():
                    continue

            if self._is_cur_stop_token():
                ret_token = self._lookup_stop_token()
                self.add_token(ret_token)
                return ret_token
            
            elif self._is_cur_keyword():
                str_repr = self.get_str_repr_of_tok(self.cur_kwd)
                for _ in range(len(str_repr)):
                    if not self._iter():
                        raise Exception("Unreachable")

                ret_token = Token(self.cur_kwd, len(self.tokens), self.cur, str_repr)
                self.add_token(ret_token)
                
                return ret_token
            else:
                t_pos = copy.copy(self.cur)
                text = self.char
                while self._iter() and not self._is_cur_stop_token() and not self.char.isspace():
                    text += self.char
                if not text:
                    raise Exception("unreachable")
                num_regex = r'\d+(\.\d*)?'
                is_num = re.fullmatch(num_regex, text)
                tok_type = None 
                if is_num:
                    if text.find('.') == -1:
                        tok_type = TokenType.T_INT
                    else:
                        tok_type = TokenType.T_FLOAT
                else:
                    if not re.fullmatch(r"[a-zA-Z]+\d*", text):
                        raise Exception(f"Tokenizer: invalid name - [{text}]")
                    tok_type = TokenType.T_WORD
                
                ret_token = Token(tok_type, len(self.tokens), t_pos, text)
                self.add_token(ret_token)
                return ret_token            
                
    def _lookup_stop_token(self) -> Token:
        match self.cur_stop:
            case TokenType.T_STR:
                str_inside = ""
                t_pos = copy.copy(self.cur)
                matched = False
                while self._iter():
                    if self.char == "\\":
                        str_inside += self.char
                        self._iter()
                        str_inside += self.char
                    if self.char == "\'":
                        matched = True
                        break
                    str_inside += self.char
                if not matched:
                    raise Exception(f"Unmatched quote at {t_pos}")
                return Token(TokenType.T_CHAR, len(self.tokens), t_pos, str_inside)

            case TokenType.T_STR:
                str_inside = ""
                t_pos = copy.copy(self.cur)
                matched = False
                while self._iter():
                    if self.char == "\\":
                        str_inside += self.char
                        self._iter()
                        str_inside += self.char
                    if self.char == "\"":
                        matched = True
                        break
                    str_inside += self.char
                if not matched:
                    raise Exception(f"Unmatched quote at {t_pos}")
                return Token(TokenType.T_STR, len(self.tokens), t_pos, str_inside)
            case _:
                t_pos = copy.copy(self.cur)
                str_repr = self.get_str_repr_of_tok(self.cur_stop)
                if not str_repr:
                    raise Exception("unreachable")
                for _ in range(len(str_repr)):
                    if not self._iter():
                        raise Exception("unreachable")
                return Token(self.cur_stop, len(self.tokens), t_pos, str_repr)

    def _is_cur_stop_token(self) -> bool:
        for str_repr, tok_type in self.STOP_TOKENS:
            if str_repr == self.text[self.cur.i-1:self.cur.i-1+len(str_repr)]:
                self.cur_stop = tok_type
                return True
        self.cur_stop = None
        return False

    def get_str_repr_of_tok(self, target: TokenType) -> str:
        for repr, tok_type in self.STOP_TOKENS:
            if tok_type == target:
                return repr
        for repr, tok_type in self.KWD_TOKENS:
            if tok_type == target:
                return repr
        
        raise Exception("Unreachable")            

    def _is_cur_keyword(self) -> bool:
        for str_repr, tok_type in self.KWD_TOKENS:
            if str_repr == self.text[self.cur.i-1:self.cur.i-1+len(str_repr)]:
                self.cur_kwd = tok_type
                return True
        self.cur_kwd = None
        return False

    def _eof(self) -> bool:
        return self.cur.i > len(self.text)

    def _iter(self) -> bool:
        if self.cur.i < 0:
            raise Exception("Invalid index access")
        if self.cur.i >= len(self.text):
            return False
        self.char = self.text[self.cur.i]
        self.cur.i += 1
        if self.char == '\n':
            self.cur.row += 1
            self.cur.col = 0
        else:
            self.cur.col += 1

        return True

    def _trim_left(self):
        if not self.char.isspace():
            return
        while self._iter():
            if not self.char.isspace():
                break
