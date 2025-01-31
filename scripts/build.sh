#/usr/bin/bash

rm -rf build/

BUILD_TYPE="Release"
BUILD_TYPE_ARG="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
TOOLCHAIN_PREFIX=cross/toolchains
OUTDIR=builds

copy_files() {
    mkdir -p $OUTDIR/$2/bin
    mkdir -p $OUTDIR/$2/lib
    cp build/$BUILD_TYPE/$1/bin/* $OUTDIR/$2/bin/
    cp build/$BUILD_TYPE/$1/lib/* $OUTDIR/$2/lib/
}

i686_linux() {
    cmake -B build $BUILD_TYPE_ARG -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_PREFIX/i686.cmake
    cmake --build build
    copy_files "Linux" "i686-linux-gnu"

    rm -rf build/
}

x86_64_linux() {
    cmake -B build $BUILD_TYPE_ARG
    cmake --build build
    copy_files "Linux" "x86_64-linux-gnu"

    rm -rf build/
}

aarch64_linux() {
    cmake -B build $BUILD_TYPE_ARG -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_PREFIX/Aarch64.cmake
    cmake --build build
    copy_files "Linux" "aarch64-linux-gnu"

    rm -rf build/
}

powerpc64_linux() {
    cmake -B build $BUILD_TYPE_ARG -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_PREFIX/Powerpc64.cmake
    cmake --build build
    copy_files "Linux" "powerpc64-linux-gnu"

    rm -rf build/
}

i686_mingw() {
    cmake -B build $BUILD_TYPE_ARG -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_PREFIX/MinGW-i686.cmake
    cmake --build build
    copy_files "Windows" "i686-w64-mingw32"

    rm -rf build/
}

x86_64_mingw() {
    cmake -B build $BUILD_TYPE_ARG -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_PREFIX/MinGW.cmake
    cmake --build build
    copy_files "Windows" "x86_64-w64-mingw32"

    rm -rf build/
}

wasm32_emscripten() {
    cmake -B build $BUILD_TYPE_ARG -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_PREFIX/Emscripten.cmake -DCMAKE_FIND_DEBUG_MODE=ON
    cmake --build build --verbose
    copy_files "Emscripten" "wasm32-unknown-emscripten"

    rm -rf build/
}

armv7_android() {
    cmake -B build \
    $BUILD_TYPE_ARG \
    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_PREFIX/Android.cmake \
    -DANDROID_ABI=armeabi-v7a \
    -DANDROID_PLATFORM=android-21

    cmake --build build

    mkdir -p $OUTDIR/armv7-linux-android/android-21/bin
    mkdir -p $OUTDIR/armv7-linux-android/android-21/lib
    cp build/Release/Android/bin/* $OUTDIR/armv7-linux-android/android-21/bin
    cp build/Release/Android/lib/* $OUTDIR/armv7-linux-android/android-21/lib

    rm -rf build/
}

aarch64_android() {
    cmake -B build \
    $BUILD_TYPE_ARG \
    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_PREFIX/Android.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-21 \
    # -DCMAKE_FIND_DEBUG_MODE=ON

    cmake --build build

    mkdir -p $OUTDIR/aarch64-linux-android/android-21/bin
    mkdir -p $OUTDIR/aarch64-linux-android/android-21/lib
    cp build/Release/Android/bin/* $OUTDIR/aarch64-linux-android/android-21/bin
    cp build/Release/Android/lib/* $OUTDIR/aarch64-linux-android/android-21/lib

    rm -rf build/
}

i386_android() {
    cmake -B build \
    $BUILD_TYPE_ARG \
    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_PREFIX/Android.cmake \
    -DANDROID_ABI=x86 \
    -DANDROID_PLATFORM=android-21

    cmake --build build

    mkdir -p $OUTDIR/i386-linux-android/android-21/bin
    mkdir -p $OUTDIR/i386-linux-android/android-21/lib
    cp build/Release/Android/bin/* $OUTDIR/i386-linux-android/android-21/bin
    cp build/Release/Android/lib/* $OUTDIR/i386-linux-android/android-21/lib

    rm -rf build/
}

x86_64_android() {
    cmake -B build \
    $BUILD_TYPE_ARG \
    -DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_PREFIX/Android.cmake \
    -DANDROID_ABI=x86_64 \
    -DANDROID_PLATFORM=android-21

    cmake --build build

    mkdir -p $OUTDIR/x86_64-linux-android/android-21/bin
    mkdir -p $OUTDIR/x86_64-linux-android/android-21/lib
    cp build/Release/Android/bin/* $OUTDIR/x86_64-linux-android/android-21/bin
    cp build/Release/Android/lib/* $OUTDIR/x86_64-linux-android/android-21/lib

    rm -rf build/
}

build_lua() {
    case $1 in
    # Linux
    'i686_linux' ) i686_linux ;;
    'x86_64_linux' ) x86_64_linux ;;
    'aarch64_linux' ) aarch64_linux ;;
    'powerpc64_linux' ) powerpc64_linux ;;
    # All linux
    'linux' )
        i686_linux
        x86_64_linux
        aarch64_linux
        powerpc64_linux ;;
    # Android
    'armv7_android' ) armv7_android ;;
    'aarch64_android' ) aarch64_android ;;
    'i386_android' ) i386_android ;;
    'x86_64_android' ) x86_64_android ;;
    # All androids
    'android' )
        armv7_android
        aarch64_android
        i386_android
        x86_64_android ;;
    # Emscripten
    'wasm32_emscripten' ) wasm32_emscripten ;;
    # MINGW
    'i686_mingw' ) i686_mingw ;;
    'x86_64_mingw' ) x86_64_mingw ;;
    # All MinGW
    'mingw' )
        i686_mingw
        x86_64_mingw ;;
    'all' )
        # Linux
        i686_linux
        x86_64_linux
        aarch64_linux
        powerpc64_linux
        # Android
        armv7_android
        aarch64_android
        i386_android
        x86_64_android
        # Emscripten
        wasm32_emscripten
        # MINGW
        i686_mingw
        x86_64_mingw ;;
    esac
}

build_lua $1