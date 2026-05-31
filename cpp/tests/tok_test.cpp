#include <iostream>
#include <tokenizer.h>


int run_test(const char* testNum, const char *expr, const std::vector<TokenType> &target, bool withDebug) {
    auto tokenizer = Tokenizer(expr);

    while (!tokenizer.eof() && !tokenizer._errBit) {
        tokenizer.next_token();
    }

    auto &actual = tokenizer._tokens;
    for (size_t i = 0; i < actual.size(); ++i) {
        if (withDebug) {
            std::println("Token: {}", actual[i]);
        }
        if (actual[i].type != target[i]) {
            std::println("Failed Test {}\nExpected at position [{}]: {}\nFound: {}\n", testNum, i, target[i], actual[i].type);
            return 69;
        }
    }

    return 0;
}

int main() {

    using enum TokenType;
    run_test(
        "1", 
        "suka123((a) + (b))*dubai /   .3123123", 
        { WORD, L_BR, L_BR, WORD, R_BR, PLUS, L_BR, WORD, R_BR, R_BR, MUL, WORD, DIV, NUM_FLOAT }, 
        true
    );

}