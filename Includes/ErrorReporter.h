#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include "Token.h"

class ErrorReporter {
    std::vector<std::string> lines;
    bool hadError = false;

public:
    ErrorReporter(const std::string& source) {
        std::stringstream ss(source);
        std::string line;
        while (std::getline(ss, line)) {
            lines.push_back(line);
        }
    }

    void error(const Token& token, const std::string& message) {
        hadError = true;
        // 1. Encabezado del error
        std::cerr << "\033[1;31m[Error]\033[0m " 
                  << "Linea " << token.line << ":" << token.column << " -> " << message << "\n";

        // 2. Snippet de código
        if (token.line > 0 && token.line <= lines.size()) {
            std::string lineStr = lines[token.line - 1];
            std::string lineNum = std::to_string(token.line);

            // MARGEN SUPERIOR: setw(4) + " | "
            // Ejemplo: "   8 | " (Total 7 caracteres)
            std::cerr << "\033[1;34m" << std::setw(4) << lineNum << " | \033[0m" << lineStr << "\n";
            
            // MARGEN INFERIOR (Corregido)
            // 4 espacios (para igualar el setw(4)) + " | "
            // Ejemplo: "     | " (Total 7 caracteres)
            std::cerr << "\033[1;34m" << "     | " << "\033[0m"; 
            
            // Espacios hasta el error (token.column es base 1)
            for (int i = 1; i < token.column; i++) std::cerr << " ";
            
            // Gusanitos
            std::cerr << "\033[1;31m^";
            // Si length es 0 o 1, al menos pintamos un ^
            int len = token.length > 0 ? token.length : 1; 
            for (int i = 1; i < len; i++) std::cerr << "~";
            
            std::cerr << "\033[0m\n";
        }
        std::cerr << "\n";
    }

    bool hasFoundError() const { return hadError; }
};