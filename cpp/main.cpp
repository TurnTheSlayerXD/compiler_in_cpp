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
    Tokenizer tokenizer("(fuu( fuu(asdasdasd), 2, 3) * (asdasdsa + 1)(69)) + ((asdasd)() + 1 / 2)");

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
    auto operand = p.or_("brace", WORD, NUM_INT, NUM_FLOAT);

    auto unOp = p.or_(p.seq(NodeType::Op_Un, opSign, operand), "call_op" /*, "subscript_op", "deref_op", "addr_op"*/)
    ->set_name("un_op");

    auto binOp = p.seq(
        NodeType::Op_Bin,
    /**/p.or_(unOp, operand), opSign, p.or_("bin_op", unOp, operand)/**/
    )->set_name("bin_op");
//3
    auto brace = p.seq( NodeType::Brace,
    /**/L_BR, p.or_(binOp, unOp, operand), R_BR/**/
    )->set_name("brace");
    (void)(brace);
//4    
    auto commaOp = p.seq( NodeType::Op_Comma,
    /**/p.or_(binOp, unOp, operand), COMMA/**/
    )->set_name("comma_op");
//5
    auto commaOpSeq = p.seq( NodeType::Op_Comma_Seq,
    /**/commaOp, p.or_("comma_op_seq", p.or_(binOp, unOp, operand))/**/
    )->set_name("comma_op_seq");
//6
    auto callBraces = p.or_ (
        p.seq(NodeType::Op_Call_Brace, 
                L_BR, p.or_(commaOpSeq, p.or_(binOp, unOp, operand)), R_BR), 
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
