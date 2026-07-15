#include <cstdio>
#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <utility>
#include <vector>
#include <cstdlib> 
#include <string>
#include <format>
#include <filesystem>
#include <unistd.h>
#include "../Includes/ErrorReporter.h"
#include "Frontend/Lexer/Lexer.h"
#include "Frontend/Parser/Parser.h"
#include "Frontend/Semantic/Semantic.h"
#include "Backend/Emitter/Emitter.h"

namespace fs = std::filesystem;

// UTILS
namespace Log {
    const std::string RESET   = "\033[0m";
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string BLUE    = "\033[34m";
    const std::string BOLD    = "\033[1m";

    void info(const std::string& msg) {
        std::cout << BLUE << BOLD << "[Ark] " << RESET << msg << std::endl;
    }

    void ok(const std::string& msg) {
        std::cout << GREEN << BOLD << "[Ark] " << RESET << msg << std::endl;
    }

    void err(const std::string& msg) {
        std::cerr << RED << BOLD << "[Ark] Error: " << RESET << msg << std::endl;
    }

    void warn(const std::string& msg) {
        std::cout << YELLOW << BOLD << "[Ark] Warning: " << RESET << msg << std::endl;
    }
}

// CLASS
class TempFile {
    private:
        std::string path;

    public:
        explicit TempFile(const std::string& data = "") {
            std::string tempPath = "/tmp/mi_app_XXXXXX";
            int fd = mkstemp(tempPath.data());

            if ( fd != -1) {
                close(fd);
                path = tempPath;

                if (!data.empty()) {
                    std::ofstream file(path);
                    if (file.is_open()) {
                        file << data;
                        file.close();
                    } else {
                        Log::err("No se pudo escribir en: " + path);
                        unlink(path.c_str());
                        path.clear();
                    }
                }
            } else {
                std::perror("Error al crear mkstemp");
            }
        }

        ~TempFile() {
            if (!path.empty()) {
                unlink(path.c_str());
                std::cout << "[Limpieza] Borrado automatico: " << path << std::endl;
            }
        }

        TempFile(const TempFile&) = delete;
        TempFile& operator=(const TempFile&) = delete;

        TempFile(TempFile&& other) noexcept : path(std::move(other.path)) {
            other.path.clear();
        }

        TempFile& operator=(TempFile&& other) noexcept {
            if (this != &other) {
                if (!path.empty()) unlink(path.c_str());
                path = std::move(other.path);
                other.path.clear();
            }
            return *this;
        }

        [[nodiscard]] std::string getPath() const { return  path; }
        [[nodiscard]] bool isValid() const { return !path.empty(); }
};

// ENTRYPOINT
int main(int argc, char* argv[]) {
    if (argc < 2) {
        Log::err("Uso: ark <archivo.ark> -O <salida>");
        return 1;
    }

    std::string inputFile = "";
    std::string outputFile = "";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o" || arg == "-O") {
            if (i + 1 < argc) {
                outputFile = argv[++i];
            } else {
                Log::err("Falta el nombre de salida despues de -o");
                return 1;
            }
        } else {
            if (inputFile.empty() && arg[0] != '-') {
                inputFile = arg;
            } else if (arg[0] == '-') {
                Log::err("Bandera desconocida: " + arg);
                return 1;
            }
        }
    }

    if (inputFile.empty()) {
        Log::err("No se especifico un archivo de entrada.");
        return 1;
    }

    if (outputFile.empty()) {
        fs::path p(inputFile);
        outputFile = p.stem().string();
    }

    // 1. Leer archivo fuente
    std::ifstream file(inputFile);
    if (!file.is_open()) {
        Log::err("No se pudo abrir el archivo " + inputFile);
        return 1;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    ErrorReporter ErrorReporter(source);

    // 2. Compilacion (ark -> ASM)
    try {
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();

        Parser parser(tokens, ErrorReporter);
        auto prog = parser.parseProgram();

        if (ErrorReporter.hasFoundError()) return 1;

        Log::info("Analizando Semantica...");
        SemanticAnalyzer semantic(ErrorReporter);
        semantic.analyze(prog);
        
        if (ErrorReporter.hasFoundError()) {
            Log::err("Fallo analisis semantico.");
            return 1;
        }

        Generator generator(prog);
        std::string asmCode = generator.generate();

        // 3. Guardar ASM temporal
        TempFile asmFile(asmCode);
        if (!asmFile.isValid()) return 1;
        Log::info(std::format("ASM generado en: {}", asmFile.getPath()));

        // 4. Invocar NASM 
        Log::info("Ensamblando objeto (NASM)...");
        TempFile objFile;
        if (!objFile.isValid()) return 1;

        Log::info("Ensamblando...");
        int nasmRet = system(std::format("nasm -f elf64 {} -o {}", asmFile.getPath(), objFile.getPath()).c_str());
        if (nasmRet != 0) {
            Log::err("NASM fallo.");
            return 1;
        }
        
        // 5. Invocar LD (Linker)
        Log::info("Linkeando...");
        int ldRet = system(std::format("ld {} -o {}", objFile.getPath(), outputFile).c_str());
        if (ldRet != 0) {
            Log::err("Linker fallo.");
            return 1;
        }
        Log::ok(std::format("EXITO: Ejecutable: ./{}", outputFile));

    } catch (const std::exception& e) { 
        Log::err(std::format("Error inesperado: {}", e.what()));
        return 1; 
    }

    return 0;
}