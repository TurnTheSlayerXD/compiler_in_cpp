// +build ignore

type Context struct {
	defined_types []string
}

func (self *Context) has_defined_type(t string) bool {
	for i:=0; i < len(self.defined_types); i += 1 {
		if self.defined_types[i] == t {
			return true
		}
	}
	return false
}


type AstNode interface {}

type TokenPos struct {
	row int
	col int
	index int
}

type ITokenProducer interface {
	next_token() (ok bool, token Token)
	get_position() TokenPos
	revert_position(t TokenPos)
	has_finished() bool 
}

type TestTokenProducer struct {
	pos int
	tokens []Token
}

func (self *TestTokenProducer) next_token() (ok bool, token Token) {
	if pos < len(tokens) {
		i += 1
		return true, tokens[i]
	}
	return false, {}
}
func new_TextTokenProducer (tokens []Token) TestTokenProducer {
	return TestTokenProducer{ pos: -1, tokens: tokens }
}

func (self *TestTokenProducer) get_position() ProducerPos {
	return { index: self.pos }
}
func (self *TestTokenProducer) revert_position(p ProducerPos) {
	if p.index > self.pos || p.index < -1 {
		panic_fmt("Passed wrond self.pos=%s", p)
	}
	self.pos = p.index
}

func (self *TestTokenProducer) has_finished() bool {
	return self.pos >= len(self.tokens)
}

type IAstBuilder interface {
	consume(tokens *ITokenProducer, ctx *Context) ( matches bool, node *AstNode)
}

type struct FuncBuilder {}
func new_FuncBuilder() *FuncBuilder {
}
func (self *FuncBuilder) consume(tokens *ITokenProducer, ctx *Context) ( matches bool, node *AstNode) {
}



type struct DeclAndAssignBuilder {}
func new_DeclAndAssignBuilder() *DeclAndAssignBuilder {
}
func (self *DeclAndAssignBuilder) consume(tokens *ITokenProducer, ctx *Context) ( matches bool, node *AstNode) {

}


type struct OuterBuilder {}

func (self *OuterBuilder) consume(tokens *ITokenProducer, ctx *Context) AstNode {
	builders := IAstBuilder[]{ new_FuncBuilder(tokens, ctx), new_DeclAndAssignBuilder(tokens, ctx) }
	for {
		has_matched_any := false
		for builder := range builders {
			pos := tokens.get_position()
			matches, node := builder.consume(tokens, ctx)
			
			if matches {
				has_matched_any = true
				break
			} else {
				tokens.revert_position(pos)
			}
		}
		
		if !has_matched_any && tokens.has_finished() {
			panic_fmt("Expected more tokens")
		}
	}
}  



type ExprId int 

const (
	Expr__Func ExprId = iota
	Expr__Type ExprId = iota

	Token__Semicolon ExprId = iota

	Op__StartOneOf ExprId = iota
	Op__EndOneOf ExprId = iota
)
