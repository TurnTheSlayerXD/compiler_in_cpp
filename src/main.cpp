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

const char *line = "\n____________________________________________________________\n";


int __main(std::string_view prog) {
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

    if (1) {
        std::printf("Tokens%s", line);
        for (auto t : tokenizer._tokens) {
            std::cout << t << std::endl;
        }
        std::printf("%sEnd Tokens%s", line, line);
    }

    Parser p;

    CExpr* mainExpr = init_parsing_expr(p);
    Node *root = p.eval(tokenizer, mainExpr);

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
    auto strTreeRepr = root->get_str_repr({.indentStep = 4, .newLine="\n"});
    std::cout << strTreeRepr;
    return 0;
}


int main(int argc, const char** argv) {
    if (argc > 1 && std::string_view(argv[1]) == std::string_view("--example")) {
        std::string_view prog = 
        "int printf(const char **fmt);"
        "void main(int argc, const char** argv) { "
        "   const int var = 1;"
        "   while(i < argc) {"
        "      if(i % 2 == 0) { "
        "          printf(\"odd\"); "
        "      }"
        "      else { "
        "          printf(\"even\");"
        "      } "
        "      i = i + 1;"
        "   }"
        "}";
        return __main(prog);
    }  
    else if (argc > 1) {
        std::ifstream str(argv[1]);
        str.seekg(0, std::ios_base::end);
        auto size = str.tellg();
        str.seekg(0, std::ios_base::beg);
        std::unique_ptr<char[]> buf(new char[size]);
        str.read(buf.get(), size);
        return __main(std::string_view(buf.get(), size));
    }
    else {
        std::string input{std::istreambuf_iterator<char>(std::cin),std::istreambuf_iterator<char>()};
        return __main(input.c_str());
    }

}