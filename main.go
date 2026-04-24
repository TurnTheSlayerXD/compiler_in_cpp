package main

type ExprId int 

const (
	Expr__Func ExprId = iota
	Expr__Type ExprId = iota

	Token__Semicolon ExprId = iota

	Op__StartOneOf ExprId = iota
	Op__EndOneOf ExprId = iota
)

type EnumToken int
const (
	T_PLUS EnumToken = iota
)

type Token struct {
	_type EnumToken
	_value *string
}


type Context struct {
	definedTypes []string
}

func (self *Context) hasDefinedType(t string) bool {
	for i:=0; i < len(self.definedTypes); i += 1 {
		if self.definedTypes[i] == t {
			return true
		}
	}



	return false
}


type AstBuilder interface {}

type Matcher interface {
	match (ctx *Context, tokens []Token) (matches bool, astBuilder *AstBuilder)
}

type FuncMatcher struct {}
func (self *FuncMatcher) match(ctx *Context, tokens []Token)  (matches bool, astBuilder *AstBuilder) { 
	
	if ok,value := ctx.getDefinedType(*tokens[0]._value) {
		
	}

	return false, nil 
}


type DeclMatcher struct {}
func (*DeclMatcher) match(ctx *Context, tokens []Token) (matches bool, astBuilder *AstBuilder) { return false, nil }


type AstNode interface {}
type Node struct {}

func process(ctx *Context, tokens []Token, matchers []Matcher) AstNode {
	j := 0
	for i := 1; i < len(tokens); i += 1 {
		for _,matcher := range matchers {
			matches, _ := matcher.match(ctx, tokens[j:i])
			if matches {
				// node := astBuilder.buildAst()
				j = i
				break
			}
		}
	}


	return Node{}
}

func get_tokens(programText string) []Token {

	return make([]Token, 0)
} 


func main() {

	programText := "int val = 1;";

	tokens := get_tokens(programText)

	ctx := Context { definedTypes: []string{}, }
	process(&ctx, tokens, []Matcher{ })
}