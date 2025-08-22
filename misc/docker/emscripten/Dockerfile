FROM emscripten/emsdk:3.1.64

ENV SOURCE_DIR="/source"
ENV BUILD_DIR="/build"
ENV OUTPUT_DIR="/dist"

# Diretório de trabalho
WORKDIR $SOURCE_DIR

COPY build.sh /build.sh
RUN chmod +x /build.sh
ENTRYPOINT [ "/build.sh" ]

# Copia o script de build
# COPY entrypoint.sh /entrypoint.sh
# RUN chmod +x /entrypoint.sh

# ENTRYPOINT ["/entrypoint.sh"]
