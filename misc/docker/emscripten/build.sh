#!/bin/bash
set -e

BUILD_DIR=/build
OUTPUT_DIR=/dist

mkdir -p $BUILD_DIR
mkdir -p $OUTPUT_DIR

echo "==> Compiling selene with Emscripten..."

cmake -S $SOURCE_DIR -B $BUILD_DIR -DCPACK_PACKAGE_DIRECTORY=$OUTPUT_DIR -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/Emscripten.cmake
cmake --build $BUILD_DIR --target package --config Release -- -j$(nproc) && \

# Copia os arquivos wasm + JS + HTML para a saída
# Supondo que o seu CMake esteja configurado para gerar game.js + game.wasm
# cp $BUILD_DIR/*.js $BUILD_DIR/*.wasm $BUILD_DIR/*.html $OUTPUT_DIR/

# Se você tiver assets opcionais
if [ -d /game ]; then
    $EMSDK/upstream/emscripten/tools/file_packager game --embed /game@./ >> /dist/game.js
fi

echo "==> Emscripten generated files in $OUTPUT_DIR"
