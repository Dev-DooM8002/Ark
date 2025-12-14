#include "../include/Parser.h"
#include <iostream>
#include <algorithm> // Para std::find

Token Parser::peek(int offset) const {
    if (pos + offset >= tokens.size()) return tokens.back();
    return tokens[pos + offset];
}

Token Parser::advance() {
    if (pos < tokens.size()) pos++;
    return peek(-1);
}

bool Parser::match(TokenType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) {
    return peek().type == type;
}

Token Parser::consume(TokenType type, std::string errorMsg) {
    if (peek().type == type) return advance();
    std::cerr << "Error Parser: " << errorMsg << " en linea " << peek().line << std::endl;
    exit(1);
}

std::shared_ptr<Program> Parser::parseProgram() {
    auto prog = std::make_shared<Program>();
    while (peek().type != TokenType::EOF_TOKEN) {
        if (match(TokenType::SEC_DATA)) parseDataSection(prog);
        else if (match(TokenType::SEC_START)) parseStartSection(prog);
        else if (match(TokenType::SEC_END)) break; 
        else advance();
    }
    return prog;
}

void Parser::parseDataSection(std::shared_ptr<Program> prog) {
    while (!check(TokenType::SEC_START) && !check(TokenType::EOF_TOKEN)) {
        prog->dataSection.push_back(statement());
    }
}

void Parser::parseStartSection(std::shared_ptr<Program> prog) {
    while (!check(TokenType::SEC_END) && !check(TokenType::EOF_TOKEN)) {
        prog->startSection.push_back(statement());
    }
    match(TokenType::SEC_END); 
}

// NUEVO: Lee bloques sin llaves, parando cuando encuentra un token de la lista
std::shared_ptr<Block> Parser::parseBlock(const std::vector<TokenType>& terminators) {
    auto block = std::make_shared<Block>();
    
    // Mientras NO encontremos uno de los terminadores y NO sea fin de archivo...
    while (std::find(terminators.begin(), terminators.end(), peek().type) == terminators.end() 
           && !check(TokenType::EOF_TOKEN)) {
        block->statements.push_back(statement());
    }
    return block;
}

std::shared_ptr<Statement> Parser::statement() {
    // --- IF STATEMENT ---
    if (match(TokenType::KW_IF)) {
        consume(TokenType::LPAREN, "Se esperaba '(' despues de if");
        auto cond = expression();
        consume(TokenType::RPAREN, "Se esperaba ')' despues de condicion");
        consume(TokenType::COLON, "Se esperaba ':' para iniciar bloque");
        
        // El bloque 'then' termina si encuentra un elif, un else, o el fin (.end)
        auto thenBlock = parseBlock({TokenType::KW_ELIF, TokenType::KW_ELSE, TokenType::SEC_END});
        auto ifStmt = std::make_shared<IfStatement>(cond, thenBlock);

        // Procesar elifs
        while (match(TokenType::KW_ELIF)) {
            consume(TokenType::LPAREN, "Se esperaba '(' despues de elif");
            auto elifCond = expression();
            consume(TokenType::RPAREN, "Se esperaba ')'");
            consume(TokenType::COLON, "Se esperaba ':'");
            
            // Un bloque elif termina si encuentra otro elif, un else, o el fin
            auto elifBlock = parseBlock({TokenType::KW_ELIF, TokenType::KW_ELSE, TokenType::SEC_END});
            ifStmt->elifs.push_back({elifCond, elifBlock});
        }

        // Procesar else
        if (match(TokenType::KW_ELSE)) {
            consume(TokenType::COLON, "Se esperaba ':' despues de else");
            // El else termina solo con el .end
            ifStmt->elseBlock = parseBlock({TokenType::SEC_END});
        }

        // AQUI ESTA LA CLAVE: Consumimos el .end obligatorio del IF completo
        consume(TokenType::SEC_END, "Se esperaba '.end' para cerrar la estructura if");
        
        return ifStmt;
    }

    if (match(TokenType::KW_VAR)) return varDeclaration();
    
    if (peek().type == TokenType::ID && peek().value == "pnl") {
        advance(); consume(TokenType::LPAREN, "("); auto e = expression(); consume(TokenType::RPAREN, ")"); match(TokenType::SEMICOLON); 
        return std::make_shared<PrintStatement>(e);
    }
    return assignmentOrExpression();
}

std::shared_ptr<Statement> Parser::varDeclaration() {
    bool isGlobal = false;
    if (match(TokenType::KW_GLOBAL)) isGlobal = true;
    match(TokenType::KW_LOCAL); 
    if (peek().type == TokenType::TYPE_INT || peek().type == TokenType::TYPE_STR) advance();

    Token name = consume(TokenType::ID, "ID requerido");
    consume(TokenType::COLON, "Expect :");
    auto init = expression();
    match(TokenType::SEMICOLON);
    return std::make_shared<VarDeclaration>(name.value, "", isGlobal, init);
}

std::shared_ptr<Statement> Parser::assignmentOrExpression() {
    if (peek().type == TokenType::ID && peek(1).type == TokenType::COLON) {
        std::string name = advance().value;
        advance();
        auto val = expression();
        match(TokenType::SEMICOLON);
        return std::make_shared<Assignment>(name, val);
    }
    auto expr = expression();
    match(TokenType::SEMICOLON);
    return nullptr; 
}

std::shared_ptr<Expression> Parser::expression() {
    auto left = term();
    while (peek().type == TokenType::EQ_EQ || peek().type == TokenType::BANG_EQ ||
           peek().type == TokenType::LT || peek().type == TokenType::GT) {
        TokenType op = advance().type;
        auto right = term();
        left = std::make_shared<BinaryExpr>(left, op, right);
    }
    return left;
}

std::shared_ptr<Expression> Parser::term() {
    auto left = primary();
    while (peek().type == TokenType::PLUS || peek().type == TokenType::MINUS) {
        TokenType op = advance().type;
        auto right = primary();
        left = std::make_shared<BinaryExpr>(left, op, right);
    }
    return left;
}

std::shared_ptr<Expression> Parser::primary() {
    if (match(TokenType::LIT_INT)) return std::make_shared<NumberLiteral>(std::stoi(peek(-1).value));
    if (match(TokenType::LIT_STR)) return std::make_shared<StringLiteral>(peek(-1).value);
    if (match(TokenType::ID)) return std::make_shared<VarReference>(peek(-1).value);
    if (match(TokenType::LPAREN)) {
        auto e = expression();
        consume(TokenType::RPAREN, ")");
        return e;
    }
    std::cerr << "Token inesperado: " << peek().value << std::endl;
    exit(1);
}