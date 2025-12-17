#include "Semantic.h"
#include <iostream>

Token makeToken(int line) {
    return {TokenType::UNKNOWN, "", line, 1, 1}; 
}

void SemanticAnalyzer::analyze(std::shared_ptr<Program> prog) {
    scopes.clear();
    enterScope();

    for (auto stmt : prog->dataSection) {
        if (auto var = std::dynamic_pointer_cast<VarDeclaration>(stmt)) {
            std::string finalType = "int";

            if (!var->type.empty()) {
                finalType = var->type;
            } else if (var->initValue) {
                finalType = checkExpression(var->initValue);
                if (finalType == "error") finalType = "int";
            }

            if (!declare(var->name, finalType)) {
                reporter.error(makeToken(var->line), "Variable global redefinida: " + var->name);
            }

            if (var->initValue) {
                std::string initType = checkExpression(var->initValue);
                if (!var->type.empty() && initType != "error" && initType != var->type) {
                     reporter.error(makeToken(var->line), "Tipo incompatible en global '" + var->name + "'. Se esperaba " + var->type + ", se obtuvo " + initType);
                }
            }
        }
    }

    for (auto stmt : prog->boxSection) {
        if (auto func = std::dynamic_pointer_cast<FunctionDef>(stmt)) {
            if (!declare(func->name, "function")) {
                 reporter.error(makeToken(func->line), "Funcion redefinida: " + func->name);
            }
        }
    }

    for (auto stmt : prog->boxSection) {
        if (auto func = std::dynamic_pointer_cast<FunctionDef>(stmt)) {
            checkFunction(func);
        }
    }

    for (auto stmt : prog->startSection) {
        checkStatement(stmt);
    }

    exitScope();
}

void SemanticAnalyzer::enterScope() {
    scopes.push_back({});
}

void SemanticAnalyzer::exitScope() {
    scopes.pop_back();
}

bool SemanticAnalyzer::declare(const std::string& name, const std::string& type) {
    if (scopes.back().count(name)) return false;
    scopes.back()[name] = type;
    return true;
}

std::string SemanticAnalyzer::resolveType(const std::string& name) {
    for (int i = scopes.size() - 1; i >= 0; i--) {
        if (scopes[i].count(name)) return scopes[i][name];
    }
    return "";
}

void SemanticAnalyzer::checkFunction(std::shared_ptr<FunctionDef> func) {
    currentFuncReturnType = func->returnType;
    enterScope();

    for (const auto& param : func->params) {
        declare(param, "int"); 
    }

    checkBlock(func->body);
    
    if (func->returnValue) {
        std::string retType = checkExpression(func->returnValue);
        if (retType != "error" && retType != currentFuncReturnType && currentFuncReturnType != "void") {
             reporter.error(makeToken(func->line), "La funcion '" + func->name + "' debe retornar " + currentFuncReturnType + " pero retorna " + retType);
        }
    }

    exitScope();
    currentFuncReturnType = "";
}

void SemanticAnalyzer::checkBlock(std::shared_ptr<Block> block) {
    for (auto stmt : block->statements) {
        checkStatement(stmt);
    }
}

void SemanticAnalyzer::checkStatement(std::shared_ptr<Statement> stmt) {
    if (auto decl = std::dynamic_pointer_cast<VarDeclaration>(stmt)) {
        std::string infType = "int";
        if (decl->initValue) {
            infType = checkExpression(decl->initValue);
        }
        
        std::string finalType = decl->type.empty() ? infType : decl->type;
        
        if (decl->initValue && infType != "error" && !decl->type.empty() && infType != decl->type) {
             reporter.error(makeToken(stmt->line), "No se puede asignar " + infType + " a variable " + finalType + " '" + decl->name + "'");
        }

        if (!declare(decl->name, finalType)) {
             reporter.error(makeToken(stmt->line), "Variable ya declarada en este bloque: " + decl->name);
        }
    }
    else if (auto assign = std::dynamic_pointer_cast<Assignment>(stmt)) {
        std::string varType = resolveType(assign->name);
        if (varType.empty()) {
            reporter.error(makeToken(stmt->line), "Variable no definida: " + assign->name);
        } else {
            std::string exprType = checkExpression(assign->value);
            if (exprType != "error" && exprType != varType) {
                 reporter.error(makeToken(stmt->line), "Asignacion incompatible. Variable '" + assign->name + "' es " + varType + ", valor es " + exprType);
            }
        }
    }
    else if (auto print = std::dynamic_pointer_cast<PrintStatement>(stmt)) {
        checkExpression(print->expression);
    }
    else if (auto pil = std::dynamic_pointer_cast<PilStatement>(stmt)) {
        checkExpression(pil->expression);
    }
    else if (auto iff = std::dynamic_pointer_cast<IfStatement>(stmt)) {
        std::string condType = checkExpression(iff->condition);
        if (condType != "error" && condType != "int") reporter.error(makeToken(stmt->line), "Condicion de IF debe ser int/bool");
        checkBlock(iff->thenBlock);
        for(auto& el : iff->elifs) { checkExpression(el.first); checkBlock(el.second); }
        if(iff->elseBlock) checkBlock(iff->elseBlock);
    }
    else if (auto loop = std::dynamic_pointer_cast<LoopStatement>(stmt)) {
        if(loop->condition) {
            std::string t = checkExpression(loop->condition);
            if(t!="error" && t!="int") reporter.error(makeToken(stmt->line), "Condicion de Loop debe ser int");
        }
        checkBlock(loop->body);
    }
    else if (auto ret = std::dynamic_pointer_cast<ExpressionStatement>(stmt)) {
        checkExpression(ret->expression);
    }
}

std::string SemanticAnalyzer::checkExpression(std::shared_ptr<Expression> expr) {
    if (auto lit = std::dynamic_pointer_cast<NumberLiteral>(expr)) return "int";
    if (auto str = std::dynamic_pointer_cast<StringLiteral>(expr)) return "str";
    
    if (auto var = std::dynamic_pointer_cast<VarReference>(expr)) {
        std::string t = resolveType(var->name);
        if (t.empty()) {
            reporter.error(makeToken(expr->line), "Variable no encontrada: " + var->name);
            return "error";
        }
        return t;
    }

    if (auto bin = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        std::string l = checkExpression(bin->left);
        std::string r = checkExpression(bin->right);
        if (l == "error" || r == "error") return "error";

        if (bin->op == TokenType::LSHIFT) {
            if (l == "str" && r == "str") return "str";
            reporter.error(makeToken(expr->line), "Concatenacion (<<) requiere dos strings. Se obtuvo " + l + " y " + r);
            return "error";
        }

        bool isMath = (bin->op == TokenType::PLUS || bin->op == TokenType::MINUS || 
                       bin->op == TokenType::STAR || bin->op == TokenType::SLASH);
        if (isMath) {
            if (l == "int" && r == "int") return "int";
            reporter.error(makeToken(expr->line), "Operacion matematica requiere ints. Se obtuvo " + l + " y " + r);
            return "error";
        }

        if (l != r) {
             reporter.error(makeToken(expr->line), "Comparacion requiere mismos tipos. " + l + " vs " + r);
             return "error";
        }
        return "int";
    }
    
    if (auto call = std::dynamic_pointer_cast<FunctionCall>(expr)) {
        return "int";
    }

    return "error";
}