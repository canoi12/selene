SDL_SOURCE_DIR="src/third/SDL2"
SDL_VERSION="2.30.11"
SDL_FILENAME="SDL2-$SDL_VERSION"
SDL_FILE="$SDL_FILENAME.tar.gz"
SDL_REPO_LINK="https://github.com/libsdl-org/SDL/releases/download/release-$SDL_VERSION/$SDL_FILE"
if [ ! -d "$SDL_SOURCE_DIR/source" ]; then
    if [ ! -d "$SDL_SOURCE_DIR" ]; then
        mkdir -p $SDL_SOURCE_DIR
    fi
    if [ ! -f $SDL_FILE ]; then
        wget "$SDL_REPO_LINK"
    fi
    tar -xvf $SDL_FILE -C "$SDL_SOURCE_DIR"
    mv "$SDL_SOURCE_DIR/$SDL_FILENAME" "$SDL_SOURCE_DIR/source"
    rm $SDL_FILE
fi
