#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Token.h"

struct ASTNode {
    virtual ~ASTNode() = default;
};

struct Expression : public ASTNode {};
struct Statement : public ASTNode {};

// --- EXPRESIONES ---
struct NumberLiteral : public Expression {
    int value;
    NumberLiteral(int v) : value(v) {}
};

struct StringLiteral : public Expression {
    std::string value;
    StringLiteral(std::string v) : value(v) {}
};

struct VarReference : public Expression {
    std::string name;
    VarReference(std::string n) : name(n) {}
};

struct BinaryExpr : public Expression {
    std::shared_ptr<Expression> left;
    TokenType op;
    std::shared_ptr<Expression> right;
    BinaryExpr(std::shared_ptr<Expression> l, TokenType o, std::shared_ptr<Expression> r)
        : left(l), op(o), right(r) {}
};

// --- SENTENCIAS ---

// Bloque de codigo (lista de sentencias entre llaves)
struct Block : public Statement {
    std::vector<std::shared_ptr<Statement>> statements;
};

// Estructura IF Ark: if ... elif ... else ...
struct IfStatement : public Statement {
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Block> thenBlock;
    
    // Lista de pares (Condicion, Bloque) para los elifs
    std::vector<std::pair<std::shared_ptr<Expression>, std::shared_ptr<Block>>> elifs;
    
    std::shared_ptr<Block> elseBlock; // Puede ser nullptr

    IfStatement(std::shared_ptr<Expression> c, std::shared_ptr<Block> t)
        : condition(c), thenBlock(t) {}
};

struct VarDeclaration : public Statement {
    std::string name;
    std::string type;
    bool isGlobal;
    std::shared_ptr<Expression> initValue;
    VarDeclaration(std::string n, std::string t, bool g, std::shared_ptr<Expression> v)
        : name(n), type(t), isGlobal(g), initValue(v) {}
};

struct Assignment : public Statement {
    std::string name;
    std::shared_ptr<Expression> value;
    Assignment(std::string n, std::shared_ptr<Expression> v)
        : name(n), value(v) {}
};

struct PrintStatement : public Statement {
    std::shared_ptr<Expression> expression;
    PrintStatement(std::shared_ptr<Expression> e) : expression(e) {}
};

struct Program : public ASTNode {
    std::vector<std::shared_ptr<Statement>> dataSection;
    std::vector<std::shared_ptr<Statement>> startSection;
};