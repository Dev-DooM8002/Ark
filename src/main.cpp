#include <iostream>
#include <fstream>  // Para leer archivos
#include <sstream>  // Para buffers de texto
#include <vector>
#include "../include/Lexer.h"
#include "../include/Parser.h"
#include "../include/Generator.h"

// Funcion helper para leer todo el texto de un archivo
std::string readFile(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error fatal: No se pudo abrir el archivo '" << filename << "'" << std::endl;
        exit(1);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    // 1. Verificar argumentos (El usuario debe pasar el archivo .ark)
    if (argc < 2) {
        std::cerr << "Uso incorrecto." << std::endl;
        std::cerr << "Sintaxis: ./build/ark <archivo.ark>" << std::endl;
        return 1;
    }

    // 2. Leer el archivo fuente
    std::string filename = argv[1];
    std::cout << "1. Leyendo archivo: " << filename << "..." << std::endl;
    std::string code = readFile(filename.c_str());

    // 3. Pipeline de Compilacion
    Lexer lexer(code);
    std::vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    std::shared_ptr<Program> program = parser.parseProgram();

    Generator generator(program);
    std::string asmCode = generator.generate();

    std::cout << "2. Generando ASM..." << std::endl;
    
    // Guardar archivo .asm (Por ahora siempre se llama output.asm en bin)
    // TODO: En el futuro, usar el mismo nombre del archivo de entrada
    std::ofstream asmFile("bin/output.asm");
    asmFile << asmCode;
    asmFile.close();

    std::cout << "3. Ensamblando y Linkeando..." << std::endl;
    
    // Llamadas al sistema para NASM y LD
    int nasmRet = system("nasm -f elf64 bin/output.asm -o bin/output.o");
    if (nasmRet != 0) {
        std::cerr << "Error en NASM assembly." << std::endl;
        return 1;
    }

    int ldRet = system("ld bin/output.o -o bin/output");
    if (ldRet != 0) {
        std::cerr << "Error en Linker." << std::endl;
        return 1;
    }
    
    std::cout << "--- Ejecutando Programa Ark ---" << std::endl;
    std::cout << "-------------------------------" << std::endl;
    
    // Ejecutamos el binario resultante
    system("./bin/output");
    
    std::cout << "\n-------------------------------" << std::endl;

    return 0;
}