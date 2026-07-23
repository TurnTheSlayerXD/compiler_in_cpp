#include <iostream>
#include <array>
#include <unordered_set>
#include <fstream>

#include <node.h>
#include <parser.h>
#include <parsing_expr.h>

#include <tokenizer.h>
#include <help.h>

#include <tree_preprocessing.h>

#include <code_builder.h> 

const char *line = "\n____________________________________________________________\n";


int main() {
    // const char *prog = "int* x = 1;";
    const char *prog = 
    "void printf(const char *fmt);"
    "int main(int argc, char** argv) { "
    "   int i = 0;"
    "   while(i < argc) {"
    "      if(i % 2 == 0) { "
    "          printf(\"even\"); "
    "      }"
    "      else { "
    "          printf(\"odd\")"
    "      } "
    "      i = i + 1;"
    "   }"
    "}";

    // const char *prog = "fu[0]((0), (1) )()()[0];";

    Tokenizer tokenizer(prog);
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

    std::printf("Tokens%s", line);
    for (auto t : tokenizer._tokens) {
        std::cout << t << std::endl;
    }
    std::printf("%sEnd Tokens%s", line, line);

    Parser p;
    Node *root = p.eval(tokenizer);

    if (!tokenizer.eof()) {
        std::cerr << "____________________________________________________________" << std::endl;
        std::cerr << "Invalid parser state: Not all tokens were consumed" << std::endl;
        if (root) {
            auto res = root->get_str_repr({.indentStep=0});
            std::cerr << res << std::endl;
        }
        std::cerr << "END of invalid parser state" << std::endl;
        std::cerr << "____________________________________________________________" << std::endl;
        return 69;
    }
    if (!root) {
        std::cout << "Invalid expr or bug in Parser!" << std::endl;
        return 69;
    }

// Processing tree
    preprocess_tree(&root);

    std::ofstream os("./__tree_ast.txt", std::ios::out);
    auto res = root->get_str_repr({.indentStep = 4, .newLine="\n"});
    os << res << std::endl;

    std::cout << res << std::endl;

    return 0;
}
