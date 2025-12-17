#pragma once
#include "../../../Includes/AST.h"
#include "../../../Includes/ErrorReporter.h"
#include <map>
#include <vector>
#include <string>
#include <memory>

class SemanticAnalyzer {
public:
    SemanticAnalyzer(ErrorReporter& reporter) : reporter(reporter) {}
    
    // Entrada principal
    void analyze(std::shared_ptr<Program> prog);

private:
    ErrorReporter& reporter;
    
    // Tabla de Simbolos: Pila de mapas (Nombre Variable -> Tipo Variable)
    // scope[0] = Globales, scope.back() = Locales actuales
    std::vector<std::map<std::string, std::string>> scopes;
    
    // Para saber si estamos dentro de una funcion y qué debe retornar
    std::string currentFuncReturnType = ""; 

    // Helpers de Scope
    void enterScope();
    void exitScope();
    bool declare(const std::string& name, const std::string& type); // Retorna false si ya existe
    std::string resolveType(const std::string& name); // Retorna el tipo o "error"

    // Visitantes (Checkers)
    void checkStatement(std::shared_ptr<Statement> stmt);
    std::string checkExpression(std::shared_ptr<Expression> expr); // Retorna el tipo resultante ("int", "str")

    // Específicos
    void checkFunction(std::shared_ptr<FunctionDef> func);
    void checkBlock(std::shared_ptr<Block> block);
};