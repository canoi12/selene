#/usr/bin/bash

rm -rf build/

ARCH=$2
BUILD_TYPE=-DCMAKE_BUILD_TYPE=Release

copy_files() {
    echo "Copying files"
    mkdir -p dist/$1$2/bin
    mkdir -p dist/$1$2/lib
    cp build/liblua5.* dist/$1/lib/
    cp build/lua dist/$1/bin
    cp build/luac dist/$1/bin
}

i686_linux() {
    cmake -B build -DCMAKE_TOOLCHAIN_FILE=../toolchains/i686.cmake $BUILD_TYPE
    cmake --build build
    # mkdir -p dist/i686-linux-gnu/bin
    # mkdir -p dist/i686-linux-gnu/lib
    # cp build/liblua5.* dist/i686-linux-gnu/lib/
    # cp build/lua dist/i686-linux-gnu/bin
    # cp build/luac dist/i686-linux-gnu/bin
    copy_files "i686-linux-gnu" ""

    rm -rf build/
}

x86_64_linux() {
    cmake -B build $BUILD_TYPE
    cmake --build build
    # mkdir -p dist/x86_64-linux-gnu/bin
    # mkdir -p dist/x86_64-linux-gnu/lib
    # cp build/liblua5.* dist/x86_64-linux-gnu/lib/
    # cp build/lua dist/x86_64-linux-gnu/bin
    # cp build/luac dist/x86_64-linux-gnu/bin
    copy_files "x86_64-linux-gnu" ""

    rm -rf build/
}

aarch64_linux() {
    cmake -B build -DCMAKE_TOOLCHAIN_FILE=../toolchains/Aarch64.cmake $BUILD_TYPE
    cmake --build build
    # mkdir -p dist/aarch64-linux-gnu/bin
    # mkdir -p dist/aarch64-linux-gnu/lib
    # cp build/liblua5.* dist/aarch64-linux-gnu/lib/
    # cp build/lua dist/aarch64-linux-gnu/bin
    # cp build/luac dist/aarch64-linux-gnu/bin
    copy_files "aarch64-linux-gnu" ""

    rm -rf build/
}

powerpc64_linux() {
    cmake -B build -DCMAKE_TOOLCHAIN_FILE=../toolchains/Powerpc64.cmake $BUILD_TYPE
    cmake --build build
    # mkdir -p dist/powerpc64-linux-gnu/bin
    # mkdir -p dist/powerpc64-linux-gnu/lib
    # cp build/liblua5.* dist/powerpc64-linux-gnu/lib/
    # cp build/lua dist/powerpc64-linux-gnu/bin
    # cp build/luac dist/powerpc64-linux-gnu/bin
    copy_files "powerpc64-linux-gnu" ""

    rm -rf build/
}

i686_mingw() {
    cmake -B build -DCMAKE_TOOLCHAIN_FILE=../toolchains/MinGW-i686.cmake $BUILD_TYPE
    cmake --build build
    mkdir -p dist/i686-w64-mingw32/bin
    mkdir -p dist/i686-w64-mingw32/lib
    cp build/liblua5* dist/i686-w64-mingw32/lib/
    cp build/*.dll dist/i686-w64-mingw32/lib/
    cp build/lua.exe dist/i686-w64-mingw32/bin
    cp build/luac.exe dist/i686-w64-mingw32/bin

    rm -rf build/
}

x86_64_mingw() {
    cmake -B build -DCMAKE_TOOLCHAIN_FILE=../toolchains/MinGW.cmake $BUILD_TYPE
    cmake --build build
    mkdir -p dist/x86_64-w64-mingw32/bin
    mkdir -p dist/x86_64-w64-mingw32/lib
    cp build/liblua5* dist/x86_64-w64-mingw32/lib/
    cp build/*.dll dist/x86_64-w64-mingw32/lib/
    cp build/lua.exe dist/x86_64-w64-mingw32/bin
    cp build/luac.exe dist/x86_64-w64-mingw32/bin

    rm -rf build/
}

wasm32_emscripten() {
    cmake -B build -DCMAKE_TOOLCHAIN_FILE=../toolchains/Emscripten.cmake $BUILD_TYPE
    cmake --build build
    # mkdir -p dist/wasm32-unknown-emscripten/bin
    # mkdir -p dist/wasm32-unknown-emscripten/lib
    # cp build/liblua5* dist/wasm32-unknown-emscripten/lib/
    # cp build/lua.* dist/wasm32-unknown-emscripten/bin
    # cp build/luac.* dist/wasm32-unknown-emscripten/bin
    copy_files "wasm32-unknown-emscripten" ""

    rm -rf build/
}

armv7_android() {
    cmake -B build \
    $BUILD_TYPE \
    -DCMAKE_TOOLCHAIN_FILE=../toolchains/Android.cmake \
    -DANDROID_ABI=armeabi-v7a \
    -DANDROID_PLATFORM=android-21

    cmake --build build

    mkdir -p dist/armv7-linux-android/android-21/bin
    mkdir -p dist/armv7-linux-android/android-21/lib
    cp build/liblua5* dist/armv7-linux-android/android-21/lib/
    cp build/lua dist/armv7-linux-android/android-21/bin
    cp build/luac dist/armv7-linux-android/android-21/bin

    rm -rf build/
}

aarch64_android() {
    cmake -B build \
    $BUILD_TYPE \
    -DCMAKE_TOOLCHAIN_FILE=../toolchains/Android.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-21

    cmake --build build

    mkdir -p dist/aarch64-linux-android/android-21/bin
    mkdir -p dist/aarch64-linux-android/android-21/lib
    cp build/liblua5* dist/aarch64-linux-android/android-21/lib/
    cp build/lua dist/aarch64-linux-android/android-21/bin
    cp build/luac dist/aarch64-linux-android/android-21/bin

    rm -rf build/
}

i386_android() {
    cmake -B build \
    $BUILD_TYPE \
    -DCMAKE_TOOLCHAIN_FILE=../toolchains/Android.cmake \
    -DANDROID_ABI=x86 \
    -DANDROID_PLATFORM=android-21

    cmake --build build

    mkdir -p dist/i386-linux-android/android-21/bin
    mkdir -p dist/i386-linux-android/android-21/lib
    cp build/liblua5* dist/i386-linux-android/android-21/lib/
    cp build/lua dist/i386-linux-android/android-21/bin
    cp build/luac dist/i386-linux-android/android-21/bin

    rm -rf build/
}

x86_64_android() {
    cmake -B build \
    $BUILD_TYPE \
    -DCMAKE_TOOLCHAIN_FILE=../toolchains/Android.cmake \
    -DANDROID_ABI=x86_64 \
    -DANDROID_PLATFORM=android-21

    cmake --build build

    mkdir -p dist/x86_64-linux-android/android-21/bin
    mkdir -p dist/x86_64-linux-android/android-21/lib
    cp build/liblua5* dist/x86_64-linux-android/android-21/lib/
    cp build/lua dist/x86_64-linux-android/android-21/bin
    cp build/luac dist/x86_64-linux-android/android-21/bin

    rm -rf build/
}

build_lua() {
    case $1 in
    # Linux
    'i686_linux' ) i686_linux ;;
    'x86_64_linux' ) x86_64_linux ;;
    'aarch64_linux' ) aarch64_linux ;;
    'powerpc64_linux' ) powerpc64_linux ;;
    # Android
    'i386_android' ) i386_android ;;
    'x86_64_android' ) x86_64_android ;;
    'armv7_android' ) armv7_android ;;
    'aarch64_android' ) aarch64_android ;;
    # Emscripten
    'wasm32_emscripten' ) wasm32_emscripten ;;
    # MinGW
    'i686_mingw' ) i686_mingw ;;
    'x86_64_mingw' ) x86_64_mingw ;;
    'all' )
        # Linux
        i686_linux
        x86_64_linux
        aarch64_linux
        powerpc64_linux
        # Android
        i386_android
        x86_64_android
        armv7_android
        aarch64_android
        # Emscripten
        wasm32_emscripten
        # MinGW
        i686_mingw
        x86_64_mingw ;;
    '*' )
        echo "Target not found"
    esac
}

build_lua $1