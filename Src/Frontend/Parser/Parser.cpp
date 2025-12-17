#include "Parser.h"
#include <iostream>
#include <algorithm> 

Token Parser::peek(int offset) const { 
    if (pos + offset >= tokens.size()) return tokens.back(); 
    return tokens[pos + offset]; 
}

Token Parser::previous() const { 
    if (pos == 0) return tokens[0]; 
    return tokens[pos - 1];
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
    if (check(type)) return advance();

    Token current = peek();
    Token prev = previous();

    if (current.line > prev.line) {
        reporter.error(prev, "Falto '" + errorMsg + "' al final de la linea");
    } else {
        reporter.error(current, "Se esperaba '" + errorMsg + "' pero se encontro: '" + current.value + "'");
    }

    throw std::runtime_error("Parser Error");
}

std::shared_ptr<Program> Parser::parseProgram() {
    auto prog = std::make_shared<Program>();
    while (peek().type != TokenType::EOF_TOKEN) {
        if (match(TokenType::SEC_DATA)) parseDataSection(prog);
        else if (match(TokenType::SEC_BOX)) parseBoxSection(prog);
        else if (match(TokenType::SEC_START)) parseStartSection(prog);
        else if (match(TokenType::SEC_END)) break; 
        else advance();
    } return prog;
}

void Parser::parseDataSection(std::shared_ptr<Program> prog) { 
    while (!check(TokenType::SEC_START) && !check(TokenType::EOF_TOKEN) && !check(TokenType::SEC_BOX)) {
        prog->dataSection.push_back(statement()); 
    }
}

void Parser::parseBoxSection(std::shared_ptr<Program> prog) {
    while (!check(TokenType::SEC_START) && !check(TokenType::SEC_DATA) && !check(TokenType::EOF_TOKEN)) {
        if (match(TokenType::KW_FN)) {
            prog->boxSection.push_back(functionDefinition());
        } else { advance(); }
    }
}

void Parser::parseStartSection(std::shared_ptr<Program> prog) { 
    while (!check(TokenType::SEC_END) && !check(TokenType::EOF_TOKEN)) {
        prog->startSection.push_back(statement()); 
    }
    match(TokenType::SEC_END); 
}

std::shared_ptr<Block> Parser::parseBlock(const std::vector<TokenType>& terminators) {
    auto block = std::make_shared<Block>();
    while (std::find(terminators.begin(), terminators.end(), peek().type) == terminators.end() && !check(TokenType::EOF_TOKEN)) {
        block->statements.push_back(statement());
    }
    return block;
}

std::shared_ptr<Statement> Parser::statement() {
    int currentLine = peek().line;

    if (match(TokenType::KW_PIL)) { 
        consume(TokenType::LPAREN, "("); 
        auto e = expression(); 
        consume(TokenType::RPAREN, ")"); 
        match(TokenType::SEMICOLON); 
        auto stmt = std::make_shared<PilStatement>(e);
        stmt->line = currentLine;
        return stmt;
    }
    if (match(TokenType::KW_CIN)) { 
        consume(TokenType::LPAREN, "("); 
        Token var = consume(TokenType::ID, "ID"); 
        std::string p = ""; 
        if (match(TokenType::COMMA)) { 
            Token msg = consume(TokenType::LIT_STR, "STR"); 
            p = msg.value; 
        } 
        consume(TokenType::RPAREN, ")"); 
        match(TokenType::SEMICOLON); 
        auto stmt = std::make_shared<InputStatement>(var.value, p);
        stmt->line = currentLine;
        return stmt;
    }
    if (peek().type == TokenType::ID && peek().value == "pnl") { 
        advance(); 
        consume(TokenType::LPAREN, "("); 
        auto e = expression(); 
        consume(TokenType::RPAREN, ")"); 
        match(TokenType::SEMICOLON); 
        auto stmt = std::make_shared<PrintStatement>(e);
        stmt->line = currentLine;
        return stmt;
    }
    
    if (match(TokenType::KW_LOOP)) {
        if (match(TokenType::COLON)) { 
            auto b = parseBlock({TokenType::SEC_END}); 
            consume(TokenType::SEC_END, ".end"); 
            auto stmt = std::make_shared<LoopStatement>(LoopType::INFINITE, nullptr, b);
            stmt->line = currentLine;
            return stmt;
        } else if (match(TokenType::LPAREN)) { 
            auto c = expression(); 
            consume(TokenType::RPAREN, ")"); 
            consume(TokenType::COLON, ":"); 
            auto b = parseBlock({TokenType::SEC_END}); 
            consume(TokenType::SEC_END, ".end"); 
            auto stmt = std::make_shared<LoopStatement>(LoopType::WHILE, c, b);
            stmt->line = currentLine;
            return stmt;
        }
    }
    if (match(TokenType::KW_ULOOP)) { 
        consume(TokenType::LPAREN, "("); 
        auto c = expression(); 
        consume(TokenType::RPAREN, ")"); 
        consume(TokenType::COLON, ":"); 
        auto b = parseBlock({TokenType::SEC_END}); 
        consume(TokenType::SEC_END, ".end"); 
        auto stmt = std::make_shared<LoopStatement>(LoopType::UNTIL, c, b);
        stmt->line = currentLine;
        return stmt;
    }
    if (match(TokenType::KW_BREAK)) { 
        match(TokenType::SEMICOLON); 
        auto stmt = std::make_shared<BreakStatement>();
        stmt->line = currentLine;
        return stmt;
    }
    if (match(TokenType::KW_JUMP)) { 
        match(TokenType::SEMICOLON); 
        auto stmt = std::make_shared<JumpStatement>();
        stmt->line = currentLine;
        return stmt;
    }

    if (match(TokenType::KW_IF)) {
        consume(TokenType::LPAREN, "("); 
        auto c = expression(); 
        consume(TokenType::RPAREN, ")"); 
        consume(TokenType::COLON, ":");
        auto tb = parseBlock({TokenType::KW_ELIF, TokenType::KW_ELSE, TokenType::SEC_END});
        auto ifs = std::make_shared<IfStatement>(c, tb);
        ifs->line = currentLine;
        while (match(TokenType::KW_ELIF)) { 
            consume(TokenType::LPAREN,"("); 
            auto ec=expression(); 
            consume(TokenType::RPAREN,")"); 
            consume(TokenType::COLON,":"); 
            ifs->elifs.push_back({ec, parseBlock({TokenType::KW_ELIF, TokenType::KW_ELSE, TokenType::SEC_END})}); 
        }
        if (match(TokenType::KW_ELSE)) { 
            consume(TokenType::COLON,":"); 
            ifs->elseBlock = parseBlock({TokenType::SEC_END}); 
        }
        consume(TokenType::SEC_END, ".end"); 
        return ifs;
    }
    if (match(TokenType::KW_VAR) || check(TokenType::TYPE_INT) || check(TokenType::TYPE_STR)) {
        return varDeclaration();
    }
    return assignmentOrExpression();
}

std::shared_ptr<Statement> Parser::varDeclaration() {
    int currentLine = peek().line;

    std::string typeStr = "";
    if (peek().type == TokenType::TYPE_INT) { typeStr = "int"; advance(); }
    else if (peek().type == TokenType::TYPE_STR) { typeStr = "str"; advance(); }
    
    Token n = consume(TokenType::ID, "ID");
    declaredVariables.insert(n.value);

    std::shared_ptr<Expression> initVal = nullptr;
    if (match(TokenType::COLON)) {
        initVal = expression();
    }
    
    match(TokenType::SEMICOLON);
    auto stmt = std::make_shared<VarDeclaration>(n.value, typeStr, false, initVal);
    stmt->line = currentLine;
    return stmt;
}

std::shared_ptr<Statement> Parser::assignmentOrExpression() {
    int currentLine = peek().line;

    if (peek().type == TokenType::ID && peek(1).type == TokenType::COLON) {
        std::string name = peek().value;
        
        if (declaredVariables.count(name)) {
            advance(); advance();
            auto v = expression();
            match(TokenType::SEMICOLON);
            auto stmt = std::make_shared<Assignment>(name, v);
            stmt->line = currentLine;
            return stmt;
        } else {
            declaredVariables.insert(name);
            advance(); advance();
            auto v = expression();
            match(TokenType::SEMICOLON);
            auto stmt = std::make_shared<VarDeclaration>(name, "", false, v);
            stmt->line = currentLine;
            return stmt;
        }
    }
    auto e = expression();
    match(TokenType::SEMICOLON);
    auto stmt = std::make_shared<ExpressionStatement>(e);
    stmt->line = currentLine;
    return stmt;
}

std::shared_ptr<Statement> Parser::functionDefinition() {
    int currentLine = peek().line;
    declaredVariables.clear();
    
    Token nameToken = consume(TokenType::ID, "Se esperaba nombre de funcion");
    
    consume(TokenType::LPAREN, "Se esperaba '('  despues del nombre.");
    std::vector<std::string> params;
    if (!check(TokenType::RPAREN)) {
        do {
            Token param = consume(TokenType::ID, "Se estepraba nombre de parametro");
            params.push_back(param.value);
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RPAREN, "Se esperaba ')' despues de parametros");

    std::string retType = "void";
    if (match(TokenType::ARROW)) {
        if (match(TokenType::TYPE_INT)) retType = "int";
        else if (match(TokenType::TYPE_STR)) retType = "str";
        else retType = "void";
    }
    consume(TokenType::COLON, "Se esperaba ':' antes del cuerpo");
    std::vector<TokenType> terminators = {
        TokenType::SEC_END,
        TokenType::SEC_START,
        TokenType::SEC_DATA,
        TokenType::SEC_BOX,
        TokenType::KW_FN
    };

    auto body = parseBlock(terminators);
    
    std::shared_ptr<Expression> retValue = nullptr;

    if (match(TokenType::SEC_END)) {
        if (match(TokenType::LPAREN)) {
            retValue = expression();
            consume(TokenType::RPAREN, "Se esperaba ')' despues del valor de retorno");
        }
    } else if (!body->statements.empty()) {
        auto lastStmt = body->statements.back();
        if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStatement>(lastStmt)) {
            retValue = exprStmt->expression;
        }
    }

    auto func = std::make_shared<FunctionDef>(nameToken.value, params, retType, body, retValue);
    func->line = currentLine;
    return func;
}

std::shared_ptr<Expression> Parser::expression() { return logicOr(); }

std::shared_ptr<Expression> Parser::logicOr() { 
    auto l = logicAnd(); 
    while (match(TokenType::KW_OR) || match(TokenType::PIPE)) {
        int line = peek(-1).line;
        auto r = logicAnd();
        l = std::make_shared<BinaryExpr>(l, TokenType::KW_OR, r); 
        l->line = line;
    }
    return l; 
}

std::shared_ptr<Expression> Parser::logicAnd() { 
    auto l = equality(); 
    while (match(TokenType::KW_AND) || match(TokenType::AMPERSAND)) {
        int line = peek(-1).line;
        auto r = equality();
        l = std::make_shared<BinaryExpr>(l, TokenType::KW_AND, r); 
        l->line = line;
    }
    return l; 
}

std::shared_ptr<Expression> Parser::equality() { 
    auto l = comparison(); 
    while (peek().type == TokenType::EQ_EQ || peek().type == TokenType::BANG_EQ) { 
        TokenType op = advance().type; 
        int line = peek(-1).line;
        auto r = comparison();
        l = std::make_shared<BinaryExpr>(l, op, r); 
        l->line = line;
    } 
    return l; 
}

std::shared_ptr<Expression> Parser::comparison() { 
    auto l = concat(); 
    while (peek().type == TokenType::LT || peek().type == TokenType::GT || peek().type == TokenType::LTE || peek().type == TokenType::GTE) { 
        TokenType op = advance().type; 
        int line = peek(-1).line;
        auto r = concat();
        l = std::make_shared<BinaryExpr>(l, op, r); 
        l->line = line;
    } 
    return l; 
}

std::shared_ptr<Expression> Parser::concat() { 
    auto l = term(); 
    while (match(TokenType::LSHIFT)) {
        int line = peek(-1).line;
        auto r = term();
        l = std::make_shared<BinaryExpr>(l, TokenType::LSHIFT, r); 
        l->line = line;
    }
    return l; 
}

std::shared_ptr<Expression> Parser::term() { 
    auto l = factor(); 
    while (peek().type == TokenType::PLUS || peek().type == TokenType::MINUS) { 
        TokenType op = advance().type; 
        int line = peek(-1).line;
        auto r = factor();
        l = std::make_shared<BinaryExpr>(l, op, r); 
        l->line = line;
    } 
    return l; 
}

std::shared_ptr<Expression> Parser::factor() { 
    auto l = unary(); 
    while (peek().type == TokenType::STAR || peek().type == TokenType::SLASH) { 
        TokenType op = advance().type; 
        int line = peek(-1).line;
        auto r = unary();
        l = std::make_shared<BinaryExpr>(l, op, r); 
        l->line = line;
    } 
    return l; 
}

std::shared_ptr<Expression> Parser::unary() { 
    if (match(TokenType::BANG) || match(TokenType::KW_NOT)) {
        int line = peek(-1).line;
        auto r = unary();
        auto u = std::make_shared<UnaryExpr>(TokenType::KW_NOT, r);
        u->line = line;
        return u;
    }
    return primary(); 
}

std::shared_ptr<Expression> Parser::primary() {
    if (match(TokenType::LIT_INT)) {
        auto e = std::make_shared<NumberLiteral>(std::stoi(peek(-1).value));
        e->line = peek(-1).line;
        return e;
    }
    if (match(TokenType::LIT_STR)) {
        auto e = std::make_shared<StringLiteral>(peek(-1).value);
        e->line = peek(-1).line;
        return e;
    }
    
    if (match(TokenType::ID)) {
        std::string name = peek(-1).value;
        int line = peek(-1).line;

        if (match(TokenType::LPAREN)) {
            std::vector<std::shared_ptr<Expression>> args;
            if (!check(TokenType::RPAREN)) {
                do {
                    args.push_back(expression());
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RPAREN, "Se esperaba ')' despues de argumentos");
            auto e = std::make_shared<FunctionCall>(name,args);
            e->line = line;
            return e;
        }
        auto e = std::make_shared<VarReference>(name);
        e->line = line;
        return e;
    }
    
    if (match(TokenType::LPAREN)) { 
        auto e = expression(); 
        consume(TokenType::RPAREN, ")"); 
        return e; 
    }
    std::cerr << "Unexpected: " << peek().value << std::endl; exit(1);
}