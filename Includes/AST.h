#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Token.h"

struct ASTNode {
    int line = 0;
    virtual ~ASTNode() = default; 
};
struct Expression : public ASTNode {};
struct Statement : public ASTNode {};

// --- EXPRESIONES ---
struct NumberLiteral : public Expression { int value; NumberLiteral(int v) : value(v) {} };
struct StringLiteral : public Expression { std::string value; StringLiteral(std::string v) : value(v) {} };
struct VarReference : public Expression { std::string name; VarReference(std::string n) : name(n) {} };
struct BinaryExpr : public Expression {
    std::shared_ptr<Expression> left; TokenType op; std::shared_ptr<Expression> right;
    BinaryExpr(std::shared_ptr<Expression> l, TokenType o, std::shared_ptr<Expression> r) : left(l), op(o), right(r) {}
};
struct UnaryExpr : public Expression { // Para NOT (!)
    TokenType op; std::shared_ptr<Expression> right;
    UnaryExpr(TokenType o, std::shared_ptr<Expression> r) : op(o), right(r) {}
};

// --- SENTENCIAS ---
struct Block : public Statement { std::vector<std::shared_ptr<Statement>> statements; };

struct IfStatement : public Statement {
    std::shared_ptr<Expression> condition; std::shared_ptr<Block> thenBlock;
    std::vector<std::pair<std::shared_ptr<Expression>, std::shared_ptr<Block>>> elifs;
    std::shared_ptr<Block> elseBlock; 
    IfStatement(std::shared_ptr<Expression> c, std::shared_ptr<Block> t) : condition(c), thenBlock(t) {}
};

enum class LoopType { INFINITE, WHILE, UNTIL };
struct LoopStatement : public Statement {
    LoopType type; std::shared_ptr<Expression> condition; std::shared_ptr<Block> body;
    LoopStatement(LoopType t, std::shared_ptr<Expression> c, std::shared_ptr<Block> b) : type(t), condition(c), body(b) {}
};

struct BreakStatement : public Statement {};
struct JumpStatement : public Statement {};
struct PilStatement : public Statement { std::shared_ptr<Expression> expression; PilStatement(std::shared_ptr<Expression> e) : expression(e) {} };
struct InputStatement : public Statement { std::string varName; std::string prompt; InputStatement(std::string v, std::string p = "") : varName(v), prompt(p) {} };

struct VarDeclaration : public Statement {
    std::string name; std::string type; bool isGlobal; std::shared_ptr<Expression> initValue;
    VarDeclaration(std::string n, std::string t, bool g, std::shared_ptr<Expression> v) : name(n), type(t), isGlobal(g), initValue(v) {}
};
struct Assignment : public Statement {
    std::string name; std::shared_ptr<Expression> value;
    Assignment(std::string n, std::shared_ptr<Expression> v) : name(n), value(v) {}
};
struct PrintStatement : public Statement {
    std::shared_ptr<Expression> expression; PrintStatement(std::shared_ptr<Expression> e) : expression(e) {}
};
struct ExpressionStatement : public Statement {
    std::shared_ptr<Expression> expression; ExpressionStatement(std::shared_ptr<Expression> e) : expression(e) {}
};

struct FunctionDef : public Statement {
    std::string name;
    std::vector<std::string> params;
    std::string returnType;
    std::shared_ptr<Block> body;
    std::shared_ptr<Expression> returnValue;

    FunctionDef(std::string n, std::vector<std::string> p, std::string rt, std::shared_ptr<Block> b, std::shared_ptr<Expression> ret)
        : name(n), params(p), returnType(rt), body(b), returnValue(ret) {}
};

struct FunctionCall : public Expression {
    std::string callee;
    std::vector<std::shared_ptr<Expression>> arguments;

    FunctionCall(std::string c, std::vector<std::shared_ptr<Expression>> args)
        : callee(c), arguments(args) {}
};

struct Program : public ASTNode {
    std::vector<std::shared_ptr<Statement>> dataSection;
    std::vector<std::shared_ptr<Statement>> boxSection;
    std::vector<std::shared_ptr<Statement>> startSection;
};