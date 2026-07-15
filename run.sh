#!/bin/bash
# Compilar rápidamente (CMake solo compilará lo que haya cambiado)
cmake --build Build
echo ""

# Ejecutar el compilador sobre el ejemplo
./Build/ark Examples/code.ark -o Bin/output

echo ""
echo "================ RUNNING PROGRAM ================"
./Bin/output
echo "================================================="