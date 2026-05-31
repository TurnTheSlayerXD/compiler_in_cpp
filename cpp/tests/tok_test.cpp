#include <iostream>
#include <tokenizer.h>

int main() {

    auto tokenizer = Tokenizer("suka123((a) + (b))*dubai /   .3123123");

    while (!tokenizer.eof() && !tokenizer._errBit) {
        Token tok = tokenizer.next_token();
    }

    std::vector<TokenType> target = { TokenType::WORD, TokenType::L_BR, TokenType::L_BR, TokenType::WORD, TokenType::R_BR, TokenType::PLUS, TokenType::L_BR, TokenType::WORD, TokenType::R_BR, TokenType::R_BR, TokenType::MUL, TokenType::WORD, TokenType::DIV, TokenType::NUM_FLOAT}; 

    auto &actual = tokenizer._tokens;
    for (size_t i = 0; i < actual.size(); ++i) {
        if (actual[i].type != target[i]) {
            std::println("Expected: {}\n\rFound\n\r", target[i], actual[i].type);
            return 69;
        }
        std::println("{}", actual[i]);
    }

}