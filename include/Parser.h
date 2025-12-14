#pragma once
#include <vector>
#include <memory>
#include "Token.h"
#include "AST.h"

class Parser {
public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens) {
        pos = 0;
    }

    std::shared_ptr<Program> parseProgram();

private:
    std::vector<Token> tokens;
    size_t pos;

    Token peek(int offset = 0) const;
    Token advance();
    bool match(TokenType type);
    bool check(TokenType type); // Nuevo helper para checar sin consumir
    Token consume(TokenType type, std::string errorMsg);

    void parseDataSection(std::shared_ptr<Program> prog);
    void parseStartSection(std::shared_ptr<Program> prog);
    
    // Ahora recibe una lista de tokens que marcan el final del bloque
    std::shared_ptr<Block> parseBlock(const std::vector<TokenType>& terminators); 

    std::shared_ptr<Statement> statement();
    std::shared_ptr<Statement> varDeclaration();
    std::shared_ptr<Statement> assignmentOrExpression();

    std::shared_ptr<Expression> expression();
    std::shared_ptr<Expression> term();
    std::shared_ptr<Expression> primary();
};