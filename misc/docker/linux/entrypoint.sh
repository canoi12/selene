#!/bin/bash
set -e

# Diretórios de build e saída
BUILD_DIR=/project/build
OUTPUT_DIR=/project/_release

mkdir -p $BUILD_DIR
mkdir -p $OUTPUT_DIR

# Configura build Release
cmake -S /project -B $BUILD_DIR -DCMAKE_BUILD_TYPE=Release -DEXPORT_APPIMAGE=TRUE

# Compila tudo
cmake --build $BUILD_DIR --config Release -- -j$(nproc)

# Executa o CPack para gerar pacotes
cd $BUILD_DIR
cpack -G DEB -B $OUTPUT_DIR
cpack -G TGZ -B $OUTPUT_DIR
cmake --build $BUILD_DIR --target package
# cpack -G AppImage -B $OUTPUT_DIR

echo "==> Generated artifacts in $OUTPUT_DIR"
