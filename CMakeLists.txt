cmake_minimum_required(VERSION 3.20)
project(Ark LANGUAGES CXX)

# Usar estándar C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Buscar todos los archivos .cpp recursivamente en Src
file(GLOB_RECURSE SOURCES "Src/*.cpp")

# Definir el ejecutable del compilador Ark
add_executable(ark ${SOURCES})

# Incluir las cabeceras
target_include_directories(ark PRIVATE Includes Src)