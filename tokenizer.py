from enum import auto, Flag

import copy

from dataclasses import dataclass

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
        raise Exception("Not implemented")

    def reset_pos(self, pos):
        raise Exception("Not implemented")

    def eof(self) -> bool:
        self.__trim_left()
        return self.__eof()

    def next_tok(self) -> Token:
        if self.tok_index < len(self.tokens):
            self.tok_index += 1
            return self.tokens[self.tok_index - 1]

        while True:
            self._trim_left()
            if self._eof():
                raise Exception("EOF Tokens: Invalid access")

            if self.char == '#':
                while self.char != '\n' and not self._eof():
                    self._iter()

            if self._is_cur_stop_token():
                ret_token = Token(self.cur_stop, len(self.tokens), self.cur)
                self.tokens.append(ret_token)
                return ret_token
            elif self._is_cur_keyword():
                ret_token = Token(self.cur_stop, len(self.tokens), self.cur)
                self.tokens.append(ret_token)
                return ret_token
            else:
                pos = self.cur
                word = ""
                while not self._is_cur_stop_token() and self._iter():
                    word += self.char
                
                
                        
    def _is_cur_stop_token(self) -> bool:
        for str_repr, tok_type in self.STOP_TOKENS:
            if str_repr == self.text[self.cur.i-1:self.cur.i-1+len(str_repr)]:
                self.cur_stop = tok_type
                return True
        self.cur_stop = None
        return False
    
    def _is_cur_keyword(self) -> bool:
        for str_repr, tok_type in self.KWD_TOKENS:
            if str_repr == self.text[self.cur.i-1:self.cur.i-1+len(str_repr)]:
                self.cur_kwd = tok_type
                return True
        self.cur_kwd = None
        return False
        
    def _eof(self) -> bool:
        return self.cur.i >= len(self.text)

    def _iter(self) -> :
        if self.cur.i < 0:
            raise Exception("Invalid index access")
        if self.cur.i + 1 > len(self.text):
            return

        self.char = self.text[self.cur.i]
        self.cur.i += 1
        if self.text[self.cur.i] == '\n':
            self.cur.row += 1
            self.cur.col = 0
        else:
            self.cur.col += 1

    def _trim_left(self):
        while self._iter():
            if not self.char.isspace():
                break
            