#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib> // Para system()
#include <string>
#include <format>
#include <unistd.h>
#include "Frontend/Lexer/Lexer.h"
#include "Frontend/Parser/Parser.h"
#include "Backend/Emitter/Emitter.h"

void rmTmp(std::vector<std::string>& files) {
    for (const std::string& file : files) {
        unlink(file.c_str());
        std::cout << "[Limpieza] Borrado: " << file << std::endl;
    }
}

std::string mkTmp(const std::string& data = "") {
    std::string tempPath = "/tmp/mi_app_XXXXXX";

    int fd = mkstemp(tempPath.data());

    if (fd != -1) { 
        close(fd);

        if (!data.empty()) {
            std::ofstream file(tempPath);
            if (!file.is_open()) {
                std::cerr << "Error: No se pudo escribir en: " << tempPath << " No se ha podido abrir." << std::endl;
                unlink(tempPath.c_str());
                return "";
            }
            file << data;
            file.close();
        }
        return tempPath;
    }
    std::perror("Error al crear mkstemp"); 
    return ""; 
}

int main(int argc, char* argv[]) {
    std::vector<std::string> tempFiles;

    if (argc < 2) {
        std::cerr << "Uso: ark <archivo.ark>" << std::endl;
        return 1;
    }

    // 1. Leer archivo fuente
    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo " << argv[1] << std::endl;
        return 1;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // 2. Compilacion (ark -> ASM)
    try {
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();
        Parser parser(tokens);
        auto prog = parser.parseProgram();
        Generator generator(prog);
        std::string asmCode = generator.generate();

        // 3. Guardar ASM temporal
        std::string asmFile = mkTmp(asmCode);
        tempFiles.push_back(asmFile);
        std::cout << "[Ark] ASM generado en: " << asmFile << std::endl;

        // 4. Invocar NASM 
        std::cout << "[Ark] Ensamblando objeto (NASM)..." << std::endl;
        std::string objFile = mkTmp();
        tempFiles.push_back(objFile);
        std::cout << "[Ark] Ensablando..." << std::endl;
        int nasmRet = system(std::format("nasm -f elf64 {} -o {}", asmFile, objFile).c_str());
        if (nasmRet != 0) {
            std::cerr << "[Ark] Error: NASM fallo." << std::endl;
            rmTmp(tempFiles);
            return 1;
        }
        
        // 5. Invocar LD (Linker)
        std::string outputExe = "output";
        std::cout << "[Ark] Linkeando..." << std::endl;
        int ldRet = system(std::format("ld {} -o {}", objFile, outputExe).c_str());
        if (ldRet != 0) {
            std::cerr << "[Ark] Error: Linker fallo." << std::endl;
            rmTmp(tempFiles);
            return 1;
        }
        std::cout << "[Ark] EXITO: Ejecutable: ./" << outputExe << std::endl;
    } catch (const std::exception e) {
        std::cerr << "[Ark] Error de compilacion: " << e.what() << std::endl;
        rmTmp(tempFiles);
        return 1;
    }

    rmTmp(tempFiles);
    return 0;
}