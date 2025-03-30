#/usr/bin/bash

OUTDIR=dist
TMPDIR="temp"
BUILDSDIR=builds/
APPIMAGETOOL=".cache/appimagetool-x86_64.AppImage"
SELENE_VERSION="0.4.1"
CROSSDIR="misc/cross"

if [ ! -d $BUILDSDIR ]; then
    ./misc/scripts/build.sh $1
fi

if [ ! -d $OUTDIR ]; then
    mkdir -p $OUTDIR
fi

generate_appimage() {
    rm -r $TMPDIR/
    mkdir $TMPDIR/
    cp -r $CROSSDIR/AppImage $TMPDIR/AppImage

    mkdir -p $TMPDIR/AppImage/usr/bin
    mkdir -p $TMPDIR/AppImage/usr/lib

    cp $BUILDSDIR/$1/bin/selene $TMPDIR/AppImage/usr/bin/selene
    cp $BUILDSDIR/$1/lib/*.so $TMPDIR/AppImage/usr/lib/

    ARCH=""
    case $1 in
    "aarch64"* ) ARCH="aarch64" ;;
    "i686"* ) ARCH="i686" ;;
    "x86_64"* ) ARCH="x86_64" ;;
    * ) echo "Invalid architecture" ;;
    esac

    ARCH=$ARCH $APPIMAGETOOL "$TMPDIR/AppImage" "$OUTDIR/Selene_v$SELENE_VERSION-$ARCH.AppImage"

    rm -r $TMPDIR
}

generate_apk() {
    rm -rf $TMPDIR/
    mkdir $TMPDIR/

    cp -r $CROSSDIR/android/* $TMPDIR/
    cd $TMPDIR
    ./gradlew build

    cd ..

    cp $TMPDIR/app/build/outputs/apk/release/*.apk "$OUTDIR/selene_v$SELENE_VERSION.apk"

    rm -r $TMPDIR
}

copy_windows() {
    if [ ! -d "$OUTDIR/windows" ]; then
        mkdir -p "$OUTDIR/windows"
        mkdir -p "$OUTDIR/windows/x64"
        mkdir -p "$OUTDIR/windows/x86"
    fi

    cp $BUILDSDIR/x86-windows-msvc/v143/bin/*.dll "$OUTDIR/windows/x86/"
    cp "$BUILDSDIR/x86-windows-msvc/v143/bin/selene.exe" "$OUTDIR/windows/x86/"
    # cp "$BUILDSDIR/x86-windows-msvc/v143/bin/SDL2.dll" "$OUTDIR/windows/x86/"
    cp ".cache/SDL2/win32/x86/SDL2.dll" "$OUTDIR/windows/x86/"
    cp ".cache/SDL2/win32/x86/README-SDL.txt" "$OUTDIR/windows/x86/"

    cp $BUILDSDIR/x64-windows-msvc/v143/bin/*.dll "$OUTDIR/windows/x64/"
    cp "$BUILDSDIR/x64-windows-msvc/v143/bin/selene.exe" "$OUTDIR/windows/x64/"
    cp ".cache/SDL2/win32/x64/SDL2.dll" "$OUTDIR/windows/x64/"
    cp ".cache/SDL2/win32/x64/README-SDL.txt" "$OUTDIR/windows/x64/"
}

copy_emscripten() {
    if [ ! -d "$OUTDIR/emscripten" ]; then
        mkdir -p "$OUTDIR/emscripten"
    fi

    cp "$BUILDSDIR/wasm32-unknown-emscripten/bin/selene.html" "$OUTDIR/emscripten/index.html"
    cp "$BUILDSDIR/wasm32-unknown-emscripten/bin/selene.js" "$OUTDIR/emscripten/"
    cp "$BUILDSDIR/wasm32-unknown-emscripten/bin/selene.wasm" "$OUTDIR/emscripten/"
}

aarch64_linux() {
    generate_appimage "aarch64-linux-gnu"
}

powerpc64_linux() {
    generate_appimage "powerpc64-linux-gnu"
}

i686_linux() {
    generate_appimage "i686-linux-gnu"
}

x86_64_linux() {
    generate_appimage "x86_64-linux-gnu"
}

case $1 in
    "emscripten" )
        copy_emscripten ;;
    "linux" )
        x86_64_linux ;;
    "windows" )
        copy_windows ;;
    "android" )
        generate_apk ;;
esac
