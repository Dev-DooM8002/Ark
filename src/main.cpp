#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "../include/Lexer.h"
#include "../include/Parser.h"
#include "../include/Generator.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: ark <archivo.ark>" << std::endl;
        return 1;
    }

    // 1. Leer archivo
    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << argv[1] << std::endl;
        return 1;
    }
    std::cout << "1. Leyendo archivo: " << argv[1] << "..." << std::endl;
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // 2. Lexer
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    // 3. Parser
    Parser parser(tokens);
    auto prog = parser.parseProgram();

    // 4. Generator
    std::cout << "2. Generando ASM..." << std::endl;
    Generator generator(prog);
    std::string asmCode = generator.generate();

    // 5. Guardar ASM
    system("mkdir -p bin");
    std::ofstream outFile("bin/output.asm");
    outFile << asmCode;
    outFile.close();

    std::cout << "3. Codigo ASM generado en bin/output.asm" << std::endl;

    return 0;
}