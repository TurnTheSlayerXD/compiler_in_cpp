
package main

import "compiler/src"

func do_test(programText string, expected []EnumToken) {
	var actual = []EnumToken{}
	tokenizer := Tokenizer_new(programText)
	i := 0
	for {
		i++
		var ok, tok = tokenizer.next_tok()
		if !ok {
			break
		}
		fmt.Printf("%d:  type:`%s`  value:`%+v` \n", i, str_EnumToken(tok.typeof), tok)
		actual = append(actual, tok.typeof)
		_ = tok
	}

	for i := 0; i < min(len(expected), len(actual)); i++ {
		if expected[i] != actual[i] {
			panic_fmt("Comparison failed at %d index.  expToken { %s } actToken { %s }", 
				i, 
				str_EnumToken(expected[i]),
				str_EnumToken(actual[i]),
			);
		}
	}
}

func test_1() {
	do_test(
		"int val = 1;",
		[]EnumToken{ T_TYPE_INT, T_CUSTOM_WORD, T_ASSIGN, T_LIT_I, T_SEMICOLON },
	)
}

func main() {
	test_1()
}