#include <iostream>
#include <array>
#include <unordered_set>

#include <node.h>
#include <parser.h>
#include <tokenizer.h>
#include <help.h>
#include <parsing_expr.h>
#include <tree_preprocessing.h>

struct TestArg {
    const char* testId; 
    const char* expr; 
    const char* expected; 
    bool debug = true;
};

const char *line = "\n____________________________________________________________\n";

int run_test(TestArg arg) {

    fprintf(stdout, "%sRunning %s%s", line, arg.testId, line);

    Tokenizer tokenizer(arg.expr);
    while (!tokenizer.eof()) {
        tokenizer.next_token();
        if (tokenizer._errBit) {
            std::cerr << "________________________________________________" << std::endl;
            std::cerr << tokenizer._errMsg << std::endl;
            std::cerr << "________________________________________________" << std::endl;
            return 69;
        }
    }
    tokenizer.reset_pos(TokPos{.index = 0});
    
    Parser p;
    Node *root = p.eval(tokenizer);

    if (!tokenizer.eof()) {
        std::fprintf(stderr, "Failed test %s%s", arg.testId, line);
        std::printf("Invalid expr or bug in Parser!\n");
        return 69;
    }
    if (!root) {
        std::fprintf(stderr, "Failed test %s%s", arg.testId, line);
        std::printf("Invalid expr or bug in Parser!\n");
        return 69;
    }

    preprocess_tree(&root);

    if (arg.debug) {
        std::printf("Tokens%s", line);
        for (auto t : tokenizer._tokens) {
            std::cout << t << std::endl;
        }
        std::printf("%sEnd Tokens%s", line, line);
    }

    if (arg.debug) {
        std::printf("%sTree%s", line, line);
        auto res = root->get_str_repr({.indentStep = 2, .newLine="\n"});
        std::cout << "`" << res << "`" << std::endl;
        std::printf("%sEnd Tree%s", line, line);
    }

    auto res = root->get_str_repr({.indentStep = 0, .newLine=""});
    if (res != arg.expected) {
        std::fprintf(stderr, "Failed test %s\n\n", arg.testId);
        std::fprintf(stderr, "Failed comparison\n\rExpected: \n`%s`\nActual:\n`%s`\n", arg.expected, res.c_str());
        return 69;
    }

    std::printf("Success %s\n", arg.testId);
    std::printf("%s\n\n", line);
    
    return 0;
}


int main() {
    // Tokenizer tokenizer("((a) + (b * d)) * (d + 69) ");

    if (run_test({
        .testId = "Test 1", 
        .expr = "int* x[] = 1;", 
        .expected = "Any: {Statement: {Var decl with assignment: {Var decl: {Typeuse: {Leaf `int`,Any: {Typespec *: {Leaf `*`,},},},Leaf `x`,Any: {Typespec []: {Leaf `[`,Leaf `]`,},},},Leaf `=`,Leaf `1`,},Leaf `;`,},},",
        .debug = true,
    }) != 0) return 69;

    if (run_test({
        .testId = "Test 2", 
        .expr = "(fuu( fuu(asdasdasd), 2, 3) * (asdasdsa + 1)(69)) + ((asdasd)() + 1 * 2);", 
        .expected = "Any: {Statement: {Bin Op: {Brace: {Leaf `(`,Bin Op: {Call op: {Leaf `fuu`,OneOrMore: {Call braces: {Leaf `(`,Comma op: {Call op: {Leaf `fuu`,OneOrMore: {Call braces: {Leaf `(`,Comma op: {Leaf `asdasdasd`,},Leaf `)`,},},},Any: {Comma op: {Leaf `,`,Leaf `2`,},Comma op: {Leaf `,`,Leaf `3`,},},},Leaf `)`,},},},Leaf `*`,Call op: {Brace: {Leaf `(`,Bin Op: {Leaf `asdasdsa`,Leaf `+`,Leaf `1`,},Leaf `)`,},OneOrMore: {Call braces: {Leaf `(`,Comma op: {Leaf `69`,},Leaf `)`,},},},},Leaf `)`,},Leaf `+`,Brace: {Leaf `(`,Bin Op: {Call op: {Brace: {Leaf `(`,Leaf `asdasd`,Leaf `)`,},OneOrMore: {Call braces: {Leaf `(`,Leaf `)`,},},},Leaf `+`,Bin Op: {Leaf `1`,Leaf `*`,Leaf `2`,},},Leaf `)`,},},Leaf `;`,},},",
        .debug = false,
    }) != 0) return 69;

    if (run_test({
        .testId = "Test 3", 
        .expr = "int* x = &*b + b[0] * 3;", 
        .expected = "Any: {Statement: {Var decl with assignment: {Var decl: {Typeuse: {Leaf `int`,Any: {Typespec *: {Leaf `*`,},},},Leaf `x`,},Leaf `=`,Bin Op: {Un Op: {Leaf `&`,Un Op: {Leaf `*`,Leaf `b`,},},Leaf `+`,Bin Op: {Call op: {Leaf `b`,OneOrMore: {Subscript: {Leaf `[`,Leaf `0`,Leaf `]`,},},},Leaf `*`,Leaf `3`,},},},Leaf `;`,},},",
        .debug = false,
    })) return 69;


    if (run_test({
        .testId = "Test 4",
        .expr = "int main(int argc, const char* aboba, int x[], char** argv) { "
            "    for (int i = 0; i < argc; i = i + 1) { "
            "       while (1) { "
            "           if(x > 4) { "
            "               print(suka); "
            "               break; "
            "           } "
            "           else { "
            "               go_fuck_urself(6)[9]; "
            "           } "
            "       } "
            "    } "
            "    continue; "
            "}",
        .expected = "FunDecl: {Typeuse: {Leaf `int`,},Leaf `main`,Call braces: {Leaf `(`,Comma op: {Var decl: {Typeuse: {Leaf `int`,},Leaf `argc`,},Any: {Comma op: {Leaf `,`,Var decl: {Typeuse: {Leaf `const`,Leaf `char`,Any: {Typespec *: {Leaf `*`,},},},Leaf `aboba`,},},Comma op: {Leaf `,`,Var decl: {Typeuse: {Leaf `int`,},Leaf `x`,Any: {Typespec []: {Leaf `[`,Leaf `]`,},},},},Comma op: {Leaf `,`,Var decl: {Typeuse: {Leaf `char`,Any: {Typespec *: {Leaf `*`,},Typespec *: {Leaf `*`,},},},Leaf `argv`,},},},},Leaf `)`,},Leaf `{`,Any: {For: {Leaf `for`,Leaf `(`,Statement: {Var decl with assignment: {Var decl: {Typeuse: {Leaf `int`,},Leaf `i`,},Leaf `=`,Leaf `0`,},Leaf `;`,},Bin Op: {Leaf `i`,Leaf `<`,Leaf `argc`,},Leaf `;`,Assignment: {Leaf `i`,Leaf `=`,Bin Op: {Leaf `i`,Leaf `+`,Leaf `1`,},},Leaf `)`,Leaf `{`,Any: {While: {Leaf `while`,Leaf `(`,Leaf `1`,Leaf `)`,Leaf `{`,Any: {IfStatement: {If: {Leaf `if`,Leaf `(`,Bin Op: {Leaf `x`,Leaf `>`,Leaf `4`,},Leaf `)`,Leaf `{`,Any: {Statement: {Call op: {Leaf `print`,OneOrMore: {Call braces: {Leaf `(`,Comma op: {Leaf `suka`,},Leaf `)`,},},},Leaf `;`,},Statement: {Leaf `break`,Leaf `;`,},},Leaf `}`,},Else: {Leaf `else`,Leaf `{`,Any: {Statement: {Call op: {Leaf `go_fuck_urself`,OneOrMore: {Call braces: {Leaf `(`,Comma op: {Leaf `6`,},Leaf `)`,},Subscript: {Leaf `[`,Leaf `9`,Leaf `]`,},},},Leaf `;`,},},Leaf `}`,},},},Leaf `}`,},},Leaf `}`,},Statement: {Leaf `continue`,Leaf `;`,},},Leaf `}`,},",
    })) return 69;


    if (run_test({
        .testId = "Test 5",
        .expr = "fu[0]((0), (1) )()()[0];",
        .expected = "Any: {Statement: {Call op: {Leaf `fu`,OneOrMore: {Subscript: {Leaf `[`,Leaf `0`,Leaf `]`,},Call braces: {Leaf `(`,Comma op: {Brace: {Leaf `(`,Leaf `0`,Leaf `)`,},Any: {Comma op: {Leaf `,`,Brace: {Leaf `(`,Leaf `1`,Leaf `)`,},},},},Leaf `)`,},Call braces: {Leaf `(`,Leaf `)`,},Call braces: {Leaf `(`,Leaf `)`,},Subscript: {Leaf `[`,Leaf `0`,Leaf `]`,},},},Leaf `;`,},},",
        .debug = false,
    })) return 69;


    if (run_test({
        .testId = "Test 6",
        .expr = "5 - x + a * d && c * 3 < 2 || (69 * 68);",
        .expected = "Any: {Statement: {Bin Op: {Bin Op: {Bin Op: {Bin Op: {Leaf `5`,Leaf `-`,Leaf `x`,},Leaf `+`,Bin Op: {Leaf `a`,Leaf `*`,Leaf `d`,},},Leaf `&&`,Bin Op: {Bin Op: {Leaf `c`,Leaf `*`,Leaf `3`,},Leaf `<`,Leaf `2`,},},Leaf `||`,Brace: {Leaf `(`,Bin Op: {Leaf `69`,Leaf `*`,Leaf `68`,},Leaf `)`,},},Leaf `;`,},},",
        .debug = false,
    }) != 0) return 69;

    return 0;
}




