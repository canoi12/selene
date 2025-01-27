#/usr/bin/bash

rm -rf build/

BUILD_TYPE="Release"
BUILD_TYPE_ARG="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"

copy_files() {
    mkdir -p dist/$2/bin
    mkdir -p dist/$2/lib
    cp build/$BUILD_TYPE/$1/bin/* dist/$2/bin/
    cp build/$BUILD_TYPE/$1/lib/* dist/$2/lib/
}

i686_linux() {
    cmake -B build $BUILD_TYPE_ARG -DCMAKE_TOOLCHAIN_FILE=../toolchains/i686.cmake
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
    cmake -B build $BUILD_TYPE_ARG -DCMAKE_TOOLCHAIN_FILE=../toolchains/Aarch64.cmake
    cmake --build build
    copy_files "Linux" "aarch64-linux-gnu"

    rm -rf build/
}

powerpc64_linux() {
    cmake -B build $BUILD_TYPE_ARG -DCMAKE_TOOLCHAIN_FILE=../toolchains/Powerpc64.cmake
    cmake --build build
    copy_files "Linux" "powerpc64-linux-gnu"

    rm -rf build/
}

i686_mingw() {
    cmake -B build $BUILD_TYPE_ARG -DCMAKE_TOOLCHAIN_FILE=../toolchains/MinGW-i686.cmake
    cmake --build build
    copy_files "Windows" "i686-w64-mingw32"

    rm -rf build/
}

x86_64_mingw() {
    cmake -B build $BUILD_TYPE_ARG -DCMAKE_TOOLCHAIN_FILE=../toolchains/MinGW.cmake
    cmake --build build
    copy_files "Windows" "x86_64-w64-mingw32"

    rm -rf build/
}

wasm32_emscripten() {
    cmake -B build $BUILD_TYPE_ARG -DCMAKE_TOOLCHAIN_FILE=../toolchains/Emscripten.cmake
    cmake --build build --verbose
    copy_files "Emscripten" "wasm32-unknown-emscripten"

    rm -rf build/
}

armv7_android() {
    cmake -B build \
    $BUILD_TYPE_ARG \
    -DCMAKE_TOOLCHAIN_FILE=../toolchains/Android.cmake \
    -DANDROID_ABI=armeabi-v7a \
    -DANDROID_PLATFORM=android-21

    cmake --build build

    mkdir -p dist/armv7-linux-android/android-21/bin
    mkdir -p dist/armv7-linux-android/android-21/lib
    cp build/Release/Android/bin/* dist/armv7-linux-android/android-21/bin
    cp build/Release/Android/lib/* dist/armv7-linux-android/android-21/lib

    rm -rf build/
}

aarch64_android() {
    cmake -B build \
    $BUILD_TYPE_ARG \
    -DCMAKE_TOOLCHAIN_FILE=../toolchains/Android.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-21

    cmake --build build

    mkdir -p dist/aarch64-linux-android/android-21/bin
    mkdir -p dist/aarch64-linux-android/android-21/lib
    cp build/Release/Android/bin/* dist/aarch64-linux-android/android-21/bin
    cp build/Release/Android/lib/* dist/aarch64-linux-android/android-21/lib

    rm -rf build/
}

i386_android() {
    cmake -B build \
    $BUILD_TYPE_ARG \
    -DCMAKE_TOOLCHAIN_FILE=../toolchains/Android.cmake \
    -DANDROID_ABI=x86 \
    -DANDROID_PLATFORM=android-21

    cmake --build build

    mkdir -p dist/i386-linux-android/android-21/bin
    mkdir -p dist/i386-linux-android/android-21/lib
    cp build/Release/Android/bin/* dist/i386-linux-android/android-21/bin
    cp build/Release/Android/lib/* dist/i386-linux-android/android-21/lib

    rm -rf build/
}

x86_64_android() {
    cmake -B build \
    $BUILD_TYPE_ARG \
    -DCMAKE_TOOLCHAIN_FILE=../toolchains/Android.cmake \
    -DANDROID_ABI=x86_64 \
    -DANDROID_PLATFORM=android-21

    cmake --build build

    mkdir -p dist/x86_64-linux-android/android-21/bin
    mkdir -p dist/x86_64-linux-android/android-21/lib
    cp build/Release/Android/bin/* dist/x86_64-linux-android/android-21/bin
    cp build/Release/Android/lib/* dist/x86_64-linux-android/android-21/lib

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
    # Emscripten
    'wasm32_emscripten' ) wasm32_emscripten ;;
    # MINGW
    'i686_mingw' ) i686_mingw ;;
    'x86_64_mingw' ) x86_64_mingw ;;
    # All androids
    'android' )
        armv7_android
        aarch64_android
        i386_android
        x86_64_android ;;
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