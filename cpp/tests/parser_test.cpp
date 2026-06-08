#include <iostream>
#include <array>
#include <unordered_set>
#include <print>

#include <node.h>
#include <parser.h>
#include <tokenizer.h>
#include <help.h>
#include <parsing_expr.h>

struct TestArg {
    const char* testId; 
    const char* expr; 
    const char* expected; 
    bool debug = true;
};

const char *line = "____________________________________________________________";

bool run_test(TestArg arg) {

    fprintf(stdout, "Running %s\n", arg.testId);
    std::fprintf(stdout, "%s\n", line);


    Tokenizer tokenizer(arg.expr);
    while (!tokenizer.eof()) {
        tokenizer.next_token();
        if (tokenizer._errBit) {
            std::fprintf(stdout, "%s\n", line);
            std::cerr << tokenizer._errMsg << std::endl;
            std::fprintf(stdout, "%s\n", line);
            break;
        }
    }
    if (arg.debug) {
        for (auto t : tokenizer._tokens) {
            std::cout << t << std::endl;
            // std::println("{}", t);
        }
    }

    tokenizer.reset_pos(TokPos{.index = 0});

    Parser p;
    auto statement = get_parsing_expr(p);

    Node *root = statement->eval(tokenizer);
    Destruct d([&root](){ delete root; });

    if (root && tokenizer.eof()) {
        auto res = root->get_str_repr({.indentStep = 0, .newLine=""});
        if (res != arg.expected) {
            std::fprintf(stderr, "Failed test %s\n\n", arg.testId);
            std::fprintf(stderr, "Failed comparison\n\rExpected: \n`%s`\nActual:\n`%s`\n", arg.expected, res.c_str());
            return false;
        }
        if (arg.debug) {
            std::fprintf(stdout, "`%s`\n", res.c_str());
        }
    }
    else if (!tokenizer.eof()) {
        std::fprintf(stderr, "Failed test %s\n\n", arg.testId);
        std::fprintf(stderr, "%s", line);
        std::fprintf(stderr, "Invalid parser state: Not all tokens were consumed\n");
        if (root) {
            auto res = root->get_str_repr();
            std::fprintf(stdout, "%s\n", res.c_str());
        }
        std::fprintf(stdout, "END of invalid parser state\n");
        std::fprintf(stdout, "%s\n", line);
        return false;
    }
    else {
        std::fprintf(stderr, "Failed test %s\n\n", arg.testId);
        std::fprintf(stderr, "%s\n", line);
        std::printf("Invalid expr or bug in Parser!\n");
        return false;
    }

    fprintf(stdout, "Success %s\n", arg.testId);
    std::fprintf(stdout, "%s\n\n", line);
    return true;
}



int main() {
    // Tokenizer tokenizer("((a) + (b * d)) * (d + 69) ");

    if (!run_test({
        .testId = "Test 1", 
        .expr = "int x = 1;", 
        .expected = "Statement: {Var decl: {Var decl: {Typeuse: {Plug,Leaf,Plug,},Leaf,Plug,},Leaf,Leaf,},Leaf,},",
        .debug = false,
    }) 
    ) return 69;

    if (!run_test({
        .testId = "Test 2", 
        .expr = "(fuu( fuu(asdasdasd), 2, 3) * (asdasdsa + 1)(69)) + ((asdasd)() + 1 * 2);", 
        .expected = "Statement: {Bin Op: {Brace: {Leaf,Bin Op: {Call op: {Leaf,Call braces: {Leaf,Comma op: {Comma op: {Call op: {Leaf,Call braces: {Leaf,Leaf,Leaf,},},Leaf,},Comma op: {Comma op: {Leaf,Leaf,},Leaf,},},Leaf,},},Leaf,Call op: {Brace: {Leaf,Bin Op: {Leaf,Leaf,Leaf,},Leaf,},Call braces: {Leaf,Leaf,Leaf,},},},Leaf,},Leaf,Brace: {Leaf,Bin Op: {Call op: {Brace: {Leaf,Leaf,Leaf,},Call braces: {Leaf,Leaf,},},Leaf,Bin Op: {Leaf,Leaf,Leaf,},},Leaf,},},Leaf,},",
        .debug = false,
    }) 
    ) return 69;

    if (!run_test({
        .testId = "Test 3", 
        .expr = "int* x = &*b + b[0] * 3;", 
        .expected = "Statement: {Var decl: {Var decl: {Typeuse: {Plug,Leaf,Leaf,},Leaf,Plug,},Leaf,Bin Op: {Un Op: {Leaf,Un Op: {Leaf,Leaf,},},Leaf,Bin Op: {Subscript op: {Leaf,Subscript: {Leaf,Leaf,Leaf,},},Leaf,Leaf,},},},Leaf,},",
        .debug = false,
    }) 
    ) return 69;


    return 0;
}




