SDL_ENV_DIR=".cache/SDL2/"
SDL_VERSION="2.30.11"
SDL_FILENAME="SDL2-$SDL_VERSION"
SDL_FILE="$SDL_FILENAME.tar.gz"
SDL_REPO_URL="https://github.com/libsdl-org/SDL/releases/download/release-$SDL_VERSION"

if [ ! -d "$SDL_ENV_DIR" ]; then
    mkdir -p $SDL_ENV_DIR
fi

download_msvc_release() {
    FILE_X64="$SDL_FILENAME-win32-x64.zip"
    FILE_X86="$SDL_FILENAME-win32-x86.zip"

    if [ ! -d "$SDL_ENV_DIR/win32" ]; then
        if [ ! -f "$SDL_ENV_DIR/$FILE_X64" ]; then
            wget "$SDL_REPO_URL/$FILE_X64" -O "$SDL_ENV_DIR/$FILE_X64"
        fi

        if [ ! -f "$SDL_ENV_DIR/$FILE_X86" ]; then
            wget "$SDL_REPO_URL/$FILE_X86" -O "$SDL_ENV_DIR/$FILE_X86"
        fi

        mkdir -p "$SDL_ENV_DIR/win32"
        mkdir -p "$SDL_ENV_DIR/win32/x64"
        mkdir -p "$SDL_ENV_DIR/win32/x86"

        unzip "$SDL_ENV_DIR/$FILE_X64" -d "$SDL_ENV_DIR/win32/x64"
        unzip "$SDL_ENV_DIR/$FILE_X86" -d "$SDL_ENV_DIR/win32/x86"
    fi    
}

download_msvc_devel() {
    FILE="SDL2-devel-$SDL_VERSION-VC.zip"
    OUTFILE="$SDL_ENV_DIR/$FILE"
    if [ ! -d "$SDL_ENV_DIR/MSVC" ]; then
        if [ ! -f $OUTFILE ]; then
            wget "$SDL_REPO_URL/$FILE" -O $OUTFILE
        fi
        unzip $OUTFILE -d "$SDL_ENV_DIR"
        mv "$SDL_ENV_DIR/$SDL_FILENAME" "$SDL_ENV_DIR/MSVC"
    fi
}

download_mingw_devel() {
    FILE="SDL2-devel-$SDL_VERSION-mingw.tar.gz"
    OUTFILE="$SDL_ENV_DIR/$FILE"
    if [ ! -d "$SDL_ENV_DIR/MINGW" ]; then
        if [ ! -f $OUTFILE ]; then
            wget "$SDL_REPO_URL/$FILE" -O $OUTFILE
        fi
        tar -xvf $OUTFILE -C "$SDL_ENV_DIR"
        mv "$SDL_ENV_DIR/$SDL_FILENAME" "$SDL_ENV_DIR/MINGW"
    fi
}

download_source() {
    FILE="$SDL_FILENAME.tar.gz"
    OUTFILE="$SDL_ENV_DIR/$FILE"
    if [ ! -d "$SDL_ENV_DIR/source" ]; then
        if [ ! -f $OUTFILE ]; then
            wget "$SDL_REPO_URL/$FILE" -O $OUTFILE
        fi
        tar -xvf $OUTFILE -C "$SDL_ENV_DIR"
        mv "$SDL_ENV_DIR/$SDL_FILENAME" "$SDL_ENV_DIR/source"
        rm $OUTFILE
    fi
}

case $1 in
    "msvc" )
        case $2 in
            "release" )
                download_msvc_release ;;
            "devel" )
                download_msvc_devel ;;
        esac ;;
    "mingw" )
        download_mingw_devel ;;
    "all" )
        download_mingw_devel
        download_msvc_devel
        download_msvc_release
        download_source ;;
    * )
        download_source ;;
esac