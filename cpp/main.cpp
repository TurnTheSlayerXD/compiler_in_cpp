#include <iostream>
#include <array>
#include <unordered_set>
#include <print>

#include <node.h>
#include <parser.h>
#include <parsing_expr.h>

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

    // const char* prog = "fuu(1, 3, bar(21321, aboba))[1] + amogus(asd );";


    // const char *prog = 
    // "int main(int argc, const char* aboba, int x[], char** argv) {"
    // "    for (int i = 0; i < argc; i = i + 1) {"
    // "        printf(fmt, argv[i]);"
    // "        printf(fmt, argv[i]);"
    // "        printf(fmt, argv[i]);"
    // "    }"
    // "}";

    const char *prog = "fu[0]((0), (1) )()()[0];";

    Tokenizer tokenizer(prog);

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

    Parser p;

    CExpr *expr = get_parsing_expr(p);
    if (!expr) {
        return 69;
    }

    Node *root = expr->eval(tokenizer);
    Destruct d([&root](){ delete root; });

    if (root && tokenizer.eof()) {
        auto res = root->get_str_repr({.indentStep = 4});
        std::cout << res << std::endl;
    }
    else if (!tokenizer.eof()) {
        
        std::cout << "____________________________________________________________" << std::endl;
        std::cout << "Invalid parser state: Not all tokens were consumed" << std::endl;

        if (root) {
            auto res = root->get_str_repr({.indentStep=0});
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
