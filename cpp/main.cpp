#include <iostream>
#include <array>
#include <unordered_set>

#include <node.h>
#include <parser.h>
#include <tokenizer.h>
#include <help.h>


int main() {
    // Tokenizer tokenizer("((a) + (b * d)) * (d + 69) ");
    // Tokenizer tokenizer("(fuu( fuu(asdasdasd), 2, 3) * (asdasdsa + 1)(69)) + ((asdasd)() + 1 * 2)");
    Tokenizer tokenizer("(fuu())()()(a())");

    while (!tokenizer.eof()) {
        tokenizer.next_token();
        if (tokenizer._errBit) {
            std::cerr << "________________________________________________" << std::endl;
            std::cerr << tokenizer._errMsg << std::endl;
            std::cerr << "________________________________________________" << std::endl;
            break;
        }
    }

    for (auto t : tokenizer._tokens) {
        std::println("{}", t);
    }

    tokenizer.reset_pos(TokPos {.index = 0});
    
    using enum TokenType;

    Parser p;
//1
    auto opSign = p.or_(PLUS, MINUS, MUL, DIV);
//2
    auto operand = p.or_("brace_op", WORD, NUM_INT, NUM_FLOAT);
//3
    auto braceOp = p.seq(
        NodeType::Op_Brace,
    /**/L_BR, p.or_("bin_op", "call_op", operand), R_BR/**/
    )->set_name("brace_op");
    (void)(braceOp);
//4    
    auto commaOp = p.seq(
        NodeType::Op_Comma,
    /**/p.or_("call_op", operand), COMMA/**/
    )->set_name("comma_op");
//5
    auto commaOpSeq = p.seq(
        NodeType::Op_Comma_Seq,
    /**/commaOp, p.or_("comma_op_seq", operand)/**/
    )->set_name("comma_op_seq");
//6
    auto callBraces = p.or_ (
        p.seq(NodeType::Op_Call_Brace, 
                L_BR, p.or_(commaOpSeq, "call_op", operand), R_BR), 
        p.seq(NodeType::Op_Call_Brace, 
                L_BR, R_BR)
    )->set_name("call_braces");
//7
    auto recursiveCallBraces = 
    /**/
        p.or_(
            p.seq (NodeType::Op_Call_Brace_Seq, 
                callBraces, 
                "recursive_call_braces"
            ),
            callBraces
        )
    /**/
    ->set_name("recursive_call_braces");
//8
    auto callOp = 
    /**/
        p.seq (
            NodeType::Op_Call,
            operand,
            recursiveCallBraces
        )
    /**/
    ->set_name("call_op");
//9
    auto binOp = p.seq(
        NodeType::Op,
    /**/p.or_(callOp, operand), opSign, p.or_("bin_op", callOp, operand)/**/
    )->set_name("bin_op");
//10
    auto expr = p.or_(binOp, callOp, operand)->set_name("expr");

//______cycle check______
    bool isCycled = p.detect_cycles(expr);
    
    if (isCycled) {
        std::cout << "________________________________________" << std::endl;
        std::cout << "WARNING: DETECTED CYCLES IN EXPR" << std::endl;
        std::cout << "________________________________________" << std::endl;
        return 69;
    }
//______end______

    Node *root = expr->eval(tokenizer);
    Destruct d([&root](){ delete root; });

    if (root && tokenizer.eof()) {
        auto res = root->get_str_repr();
        std::cout << res << std::endl;
    }
    else if (!tokenizer.eof()) {
        std::cout << "Invalid parser state: Not all tokens were consumed" << std::endl;
    }
    else {
        std::cout << "Invalid expr or bug in Parser!" << std::endl;
    }
}
