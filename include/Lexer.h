#pragma once
#include <string>
#include <vector>
#include "Token.h"

class Lexer {
public:
    Lexer(std::string source) : src(source) {
        pos = 0;
        line = 1;
    }

    std::vector<Token> tokenize();

private:
    std::string src;
    size_t pos;
    int line;

    char peek(int offset = 0) const;
    char advance();
    bool isAtEnd() const;
    
    Token makeToken(TokenType type, std::string value);
    Token identifier();
    Token number();
    Token stringLit();
};