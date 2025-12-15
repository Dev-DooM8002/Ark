#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib> // Para system()
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
    std::cout << "[Ark] 1. Leyendo archivo: " << argv[1] << "..." << std::endl;
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // 2. Pipeline (Lexer -> Parser -> Generator)
    Lexer lexer(source);
    std::vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    auto prog = parser.parseProgram();

    std::cout << "[Ark] 2. Generando ASM..." << std::endl;
    Generator generator(prog);
    std::string asmCode = generator.generate();

    // 3. Guardar ASM
    system("mkdir -p bin");
    std::ofstream outFile("bin/output.asm");
    outFile << asmCode;
    outFile.close();

    // 4. Invocar NASM (Ensamblador)
    std::cout << "[Ark] 3. Ensamblando objeto (NASM)..." << std::endl;
    int nasmRet = system("nasm -f elf64 bin/output.asm -o bin/output.o");
    if (nasmRet != 0) {
        std::cerr << "[Ark] Error: Fallo en NASM." << std::endl;
        return 1;
    }

    // 5. Invocar LD (Linker)
    std::cout << "[Ark] 4. Linkeando ejecutable (LD)..." << std::endl;
    int ldRet = system("ld bin/output.o -o bin/output");
    if (ldRet != 0) {
        std::cerr << "[Ark] Error: Fallo en el Linker." << std::endl;
        return 1;
    }

    std::cout << "[Ark] EXITO: Ejecutable generado en 'bin/output'" << std::endl;
    return 0;
}