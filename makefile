CXX = g++
CXXFLAGS = -std=c++20 -IIncludes -ISrc
SRC = $(wildcard Src/*.cpp Src/*/*/*.cpp)
COMPILER = build/ark
EXAMPLE = Examples/code.ark
OUTPUT = output

run: $(COMPILER)
	./$(COMPILER) $(EXAMPLE)
	@echo ""
	@echo "---- EJECUTANDO PROGRAMA ----"
	./$(OUTPUT)
	@echo "-----------------------------"

$(COMPILER): $(SRC)
	@mkdir -p build
	$(CXX) $(SRC) -o $(COMPILER) $(CXXFLAGS)

clean:
	rm -rf build $(OUTPUT)

.PHONY: run clean