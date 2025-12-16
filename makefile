# Variables calcadas a tu script
CXX = g++
CXXFLAGS = -std=c++20
SRC = src/*.cpp
COMPILER = build/ark
EXAMPLE = examples/code.ark
OUTPUT = output

# Regla principal: 'make run' o solo 'make' hace todo
run: $(COMPILER)
	@echo "[Make] Ejecutando Ark sobre $(EXAMPLE)..."
	./$(COMPILER) $(EXAMPLE)
	@echo ""
	@echo "--- EJECUTANDO PROGRAMA DE USUARIO ---"
	@echo "--------------------------------------"
	./$(OUTPUT)
	@echo "--------------------------------------"

# Regla de compilación (Solo recompila si modificaste los .cpp)
$(COMPILER): $(wildcard src/*.cpp)
	@echo "[Make] Compilando Ark (C++)..."
	@mkdir -p build
	$(CXX) $(SRC) -o $(COMPILER) $(CXXFLAGS)

# Limpieza (igual a tu paso 1 pero manual)
clean:
	@echo "[Make] Limpiando directorios..."
	rm -rf build $(OUTPUT)

.PHONY: run clean