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

    // utilidades
    Token peek(int offset = 0) const;
    Token advance();
    bool match(TokenType type);
    bool check(TokenType type); 
    Token consume(TokenType type, std::string errorMsg);

    void parseDataSection(std::shared_ptr<Program> prog);
    void parseStartSection(std::shared_ptr<Program> prog);
    
    std::shared_ptr<Block> parseBlock(const std::vector<TokenType>& terminators); 

    // Nodos
    std::shared_ptr<Statement> statement();
    std::shared_ptr<Statement> varDeclaration();
    std::shared_ptr<Statement> assignmentOrExpression();

    // --- JERARQUIA DE EXPRESIONES (PEMDAS) ---
    std::shared_ptr<Expression> expression();   // logicOr
    std::shared_ptr<Expression> logicOr();      // ||
    std::shared_ptr<Expression> logicAnd();     // &&
    std::shared_ptr<Expression> equality();     // == !=
    std::shared_ptr<Expression> comparison();   // < > <= >=
    std::shared_ptr<Expression> concat();       // <<
    std::shared_ptr<Expression> term();         // + -
    std::shared_ptr<Expression> factor();       // * /
    std::shared_ptr<Expression> unary();        // ! -
    std::shared_ptr<Expression> primary();      // literal, id, (expr)
};