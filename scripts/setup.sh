#!/bin/bash
CACHE_DIR=".cache"

setup_android() {
    CMDLT_FILE="commandlinetools-linux-11076708_latest.zip"
    ANDROID_PLATFORM=21
    NDK_VERSION="25.2.9519653"
    BUILD_TOOLS_VERSION="34.0.0"
    CMDLINE_TOOLS_URL="https://dl.google.com/android/repository/$CMDLT_FILE"

    OUTDIR="$CACHE_DIR/AndroidSDK"

    if [ ! -f "$CACHE_DIR/$CMDLT_FILE" ]; then
        wget $CMDLINE_TOOLS_URL -O "$CACHE_DIR/$CMDLT_FILE"
    fi

    if [ ! -d "$OUTDIR/cmdline-tools/" ]; then
        mkdir -p "$OUTDIR/cmdline-tools/"
        unzip "$CACHE_DIR/$CMDLT_FILE" -d "$OUTDIR/cmdline-tools/"
        mv "$OUTDIR/cmdline-tools/cmdline-tools" "$OUTDIR/cmdline-tools/latest"
    fi

    SDK_MANAGER="$OUTDIR/cmdline-tools/latest/bin/sdkmanager"
    if [ ! -d "$OUTDIR/platform-tools" ]; then
        $SDK_MANAGER "platform-tools"
    fi

    if [ ! -d "$OUTDIR/platforms/android-$ANDROID_PLATFORM" ]; then
        $SDK_MANAGER "platforms;android-$ANDROID_PLATFORM"
    fi

    if [ ! -d "$OUTDIR/ndk/$NDK_VERSION" ]; then
        $SDK_MANAGER --install "ndk;$NDK_VERSION"
    fi

    if [ ! -d "$OUTDIR/build-tools" ]; then
        $SDK_MANAGER --install "build-tools;$BUILD_TOOLS_VERSION"
    fi
}

setup_emscripten() {
    if [ ! -d "$CACHE_DIR/emsdk" ]; then
        git clone "https://github.com/emscripten-core/emsdk.git" "$CACHE_DIR/emsdk"
    fi
    cd $CACHE_DIR/emsdk
    ./emsdk install latest
    ./emsdk activate latest
}

case $1 in
    "android" )
        setup_android ;;
    "emscripten" )
        setup_emscripten ;;
esac