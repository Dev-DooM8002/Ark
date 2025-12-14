#pragma once
#include "AST.h"
#include <sstream>
#include <map>

class Generator {
public:
    Generator(std::shared_ptr<Program> prog) : prog(prog) {}

    std::string generate();

private:
    std::shared_ptr<Program> prog;
    std::stringstream output;
    
    std::map<std::string, int> localVars;
    int stackOffset = 0; 
    int stringCount = 0;
    int labelCounter = 0; // Para etiquetas IF

    void emit(std::string code);
    void push(std::string reg);
    void pop(std::string reg);

    void genStatement(std::shared_ptr<Statement> stmt);
    void genExpression(std::shared_ptr<Expression> expr);
};