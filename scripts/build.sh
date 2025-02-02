#/usr/bin/bash

rm -rf build/

BUILD_TYPE="Release"
BUILD_TYPE_ARG="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
TOOLCHAINS_PREFIX=cmake/toolchains
OUTDIR=builds

copy_files() {
    mkdir -p $OUTDIR/$1/bin
    mkdir -p $OUTDIR/$1/lib
    cp build/$BUILD_TYPE/bin/* $OUTDIR/$1/bin/
    cp build/$BUILD_TYPE/lib/* $OUTDIR/$1/lib/
}

build_default() {
    cmake -B build -DCMAKE_BUILD_TYPE=$BUILD_TYPE
    cmake --build build
}

build_cross_linux() {
    TOOLCHAIN_FILE=""
    TRIPLE=""
    case $1 in
        "i686" )
            TOOLCHAIN_FILE="-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAINS_PREFIX/i686.cmake"
            TRIPLE="i686-linux-gnu" ;;
        "aarch64" )
            TOOLCHAIN_FILE="-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAINS_PREFIX/Aarch64.cmake"
            TRIPLE="aarch64-linux-gnu" ;;
        "powerpc64" )
            TOOLCHAIN_FILE="-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAINS_PREFIX/Powerpc64.cmake"
            TRIPLE="powerpc64-linux-gnu" ;;
    esac

    cmake -B build -DCMAKE_BUILD_TYPE=$BUILD_TYPE $TOOLCHAIN_FILE
    cmake --build build

    copy_files $TRIPLE
    rm -rf build
}

build_linux() {
    build_default
    copy_files "x86_64-linux-gnu"
    rm -rf build
}

build_mingw() {
    TOOLCHAIN_FILE=""
    TRIPLE=""
    case $1 in
        "i686" )
            TOOLCHAIN_FILE="-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAINS_PREFIX/MinGW-i686.cmake"
            TRIPLE="i686-w64-mingw32" ;;
        * )
            TOOLCHAIN_FILE="-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAINS_PREFIX/MinGW.cmake"
            TRIPLE="x86_64-w64-mingw32" ;;
    esac

    cmake -B build -DCMAKE_BUILD_TYPE=$BUILD_TYPE $TOOLCHAIN_FILE
    cmake --build build

    copy_files $TRIPLE

    rm -rf build
}

build_android() {
    ANDROID_ABI=""
    ANDROID_PLATFORM=""
    ANDROID_ARCH=$1

    case $1 in
        "armv7" )
            ANDROID_ABI="armeabi-v7a" ;;
        "aarch64" )
            ANDROID_ABI="arm64-v8a" ;;
        "i386" )
            ANDROID_ABI="x86" ;;
        "x86_64" )
            ANDROID_ABI="x86_64" ;;
        * )
            ANDROID_ABI="x86_64"
            ANDROID_ARCH="aarch64" ;;
    esac

    case $2 in
        "android-*" )
            ANDROID_PLATFORM=$2 ;;
        * )
            ANDROID_PLATFORM="android-21" ;;
    esac

    echo "Generating files"
    cmake -B build \
    $BUILD_TYPE_ARG \
    -DCMAKE_TOOLCHAIN_FILE=".cache/AndroidSDK/ndk/25.2.9519653/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI=$ANDROID_ABI \
    -DANDROID_PLATFORM=$ANDROID_PLATFORM \

    echo "Building project"
    cmake --build build

    mkdir -p $OUTDIR/$ANDROID_ARCH-linux-android/$ANDROID_PLATFORM/bin
    mkdir -p $OUTDIR/$ANDROID_ARCH-linux-android/$ANDROID_PLATFORM/lib
    cp build/$BUILD_TYPE/bin/* $OUTDIR/$ANDROID_ARCH-linux-android/$ANDROID_PLATFORM/bin
    cp build/$BUILD_TYPE/lib/* $OUTDIR/$ANDROID_ARCH-linux-android/$ANDROID_PLATFORM/lib

    rm -rf build/
}

build_emscripten() {
    cmake -B build $BUILD_TYPE_ARG -DCMAKE_TOOLCHAIN_FILE=.cache/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
    cmake --build build --verbose
    copy_files "wasm32-unknown-emscripten"

    rm -rf build/
}

case $1 in
    "emscripten" )
        build_emscripten ;;
    "cross" )
        build_cross_linux $2 ;;
    "android" )
        build_android $2 $3 ;;
    "mingw" )
        build_mingw $2 ;;
    "linux" )
        build_linux ;;
    * )
        build_default
        copy_files "default" ;;
esac