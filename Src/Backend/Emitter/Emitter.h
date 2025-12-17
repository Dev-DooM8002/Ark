#pragma once
#include "../../../Includes/AST.h"
#include <sstream>
#include <map>
#include <vector>

// Estructura para saber que onda con cada variable
struct VarInfo {
    int offset;      // Donde vive en el stack (rbp - offset)
    std::string type; // "int" o "str"
};

class Generator {
public:
    Generator(std::shared_ptr<Program> prog) : prog(prog) {}

    std::string generate();

private:
    std::shared_ptr<Program> prog;
    std::stringstream output;
    
    // Mapa actualizado: Nombre -> Info (Offset + Tipo)
    std::map<std::string, VarInfo> localVars;
    std::map<std::string, std::string> globalVars;
    
    int stackOffset = 0; 
    int stringCount = 0;
    int labelCounter = 0; 

    std::vector<std::pair<std::string, std::string>> loopStack;
    std::vector<std::pair<std::string, std::string>> stringLiterals;

    void emit(std::string code);
    void push(std::string reg);
    void pop(std::string reg);

    void collectStrings(std::shared_ptr<Statement> stmt);
    void collectStringsFromExpr(std::shared_ptr<Expression> expr);

    void genStatement(std::shared_ptr<Statement> stmt);
    void genExpression(std::shared_ptr<Expression> expr);

    void genFunctionDef(std::shared_ptr<FunctionDef> funct);
    void genFunctionCall(std::shared_ptr<FunctionCall> call);

    // NUEVO: El cerebro que adivina tipos
    std::string inferType(std::shared_ptr<Expression> expr);
    std::map<std::string, int> currentFuncArgs;
};