FROM ubuntu:22.04

ENV SOURCE_DIR="/source"
ENV BUILD_DIR="/build"
ENV OUTPUT_DIR="/dist"
ENV ARCH="$(uname -m)"

RUN ARCH=$(uname -m)
RUN echo "Architecture: $ARCH"

# Instala dependências de build e empacotamento
RUN apt-get update && apt-get install -y \
    build-essential fuse file libfuse2 cmake git wget

RUN apt-get install -y libsdl2-dev
RUN rm -rf /var/lib/apt/lists/*

# Cria diretório de trabalho
WORKDIR $SOURCE_DIR

RUN mkdir -p $BUILD_DIR
RUN mkdir -p $OUTPUT_DIR

COPY ./DownloadAppImage.cmake /DownloadAppImage.cmake

RUN cmake -DTARGET_ARCHITECTURE=$ARCH -P /DownloadAppImage.cmake
# RUN modprobe fuse
CMD cmake -B $BUILD_DIR -DCPACK_PACKAGE_DIRECTORY=$OUTPUT_DIR -DCMAKE_BUILD_TYPE=Release -DEXPORT_APPIMAGE=ON && cmake --build $BUILD_DIR --target package

# Copia o script de build
# COPY entrypoint.sh /entrypoint.sh
# RUN chmod +x /entrypoint.sh

# ENTRYPOINT ["/entrypoint.sh"]
