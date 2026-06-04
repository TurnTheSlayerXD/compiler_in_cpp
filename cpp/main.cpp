#include <iostream>
#include <array>
#include <unordered_set>
#include <print>

#include <node.h>
#include <parser.h>
#include <tokenizer.h>
#include <help.h>


int main() {
    // Tokenizer tokenizer("((a) + (b * d)) * (d + 69) ");
    // Tokenizer tokenizer("(fuu( fuu(asdasdasd), 2, 3) * (asdasdsa + 1)(69)) + ((asdasd)() + 1 * 2)");

    // if (argc < 2) {
    //     std::cerr << "No args were provided" << std::endl;
    //     return 69;
    // }
    // const char *str = argv[1];

    Tokenizer tokenizer("int* x = 1");

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
        std::cout << t << std::endl;
        // std::println("{}", t);
    }

    tokenizer.reset_pos(TokPos{.index = 0});
    
    using enum TokenType;

    Parser p;
//1
    auto opSign = p.or_(PLUS, MINUS, MUL, DIV, GR, LE, GR_E, LE_E, EQ, AND, OR);
//2
    auto operand = p.or_("brace", WORD, NUM_INT, NUM_FLOAT);

    auto unOp = p.or_(
        "call_op", 
        "subscr_op",
        p.seq(NodeType::Op_Un, p.or_(PLUS, MINUS, MUL, ADDR, INCR, DECR), p.or_("un_op", operand))
    )
    ->set_name("un_op");

    auto binOp = p.seq(
        NodeType::Op_Bin,
    /**/p.or_(unOp, operand), opSign, "rvalue"/**/
    )->set_name("bin_op");
//3
    auto brace = p.seq( NodeType::Brace,
    /**/L_BR, "expr", R_BR/**/
    )->set_name("brace");
    (void)(brace);
//4    
    auto commaOp = p.seq( NodeType::Op_Comma,
    /**/"rvalue", COMMA/**/
    )->set_name("comma_op");
//5
    auto commaOpSeq = p.seq( NodeType::Op_Comma_Seq,
    /**/commaOp, p.or_("comma_op_seq", "rvalue")/**/
    )->set_name("comma_op_seq");
//6
    auto callBraces = p.or_ (
        p.seq(NodeType::Op_Call_Brace, 
                L_BR, p.or_(commaOpSeq, "rvalue"), R_BR), 
        p.seq(NodeType::Op_Call_Brace, 
                L_BR, R_BR)
    )->set_name("call_braces");
//7
    auto seqCallBraces = p.or_(
        p.seq (NodeType::Op_Call_Brace_Seq, 
            callBraces, 
            p.or_("seq_call_braces", "seq_subscr")
        ),
        callBraces
    )
    ->set_name("seq_call_braces");
//8
    auto callOp = 
        p.seq (
            NodeType::Op_Call,
            p.or_("subscr_op", operand),
            seqCallBraces
        )
    ->set_name("call_op"); 
    (void)(callOp);

    auto subscr = p.seq(
        NodeType::Subscr,
        L_SUBSCR, "rvalue", R_SUBSCR
    );

    auto seqSubscr = p.or_(
        p.seq (NodeType::Subscr_Seq, 
            subscr, p.or_("seq_subscr", seqCallBraces)
        ),
        subscr
    )
    ->set_name("seq_subscr");
//9
    auto subscrOp = 
        p.seq (
            NodeType::Op_Subscr,
            operand, seqSubscr
        )
    ->set_name("subscr_op");
    (void)(subscrOp);
//10
    auto rvalue = p.or_(binOp, unOp, operand)
    ->set_name("rvalue");

    auto lvalue = p.or_(p.seq(NodeType::Lvalue, MUL, p.or_(unOp, operand)), operand);

    auto assign = p.seq(NodeType::Assignment, lvalue, ASSIGN, rvalue);

    auto expr = p.or_(assign, rvalue, lvalue)
        ->set_name("expr");
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
        
        std::cout << "____________________________________________________________" << std::endl;
        std::cout << "Invalid parser state: Not all tokens were consumed" << std::endl;

        if (root) {
            auto res = root->get_str_repr();
            std::cout << res << std::endl;
        }

        std::cout << "END of invalid parser state" << std::endl;
        std::cout << "____________________________________________________________" << std::endl;

        return 69;
    }
    else {
        std::cout << "Invalid expr or bug in Parser!" << std::endl;
        return 69;
    }

    return 0;
}
