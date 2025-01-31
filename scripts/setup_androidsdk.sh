CMDLT_FILE="commandlinetools-linux-11076708_latest.zip"
ANDROID_PLATFORM=21
NDK_VERSION="25.2.9519653"
BUILD_TOOLS_VERSION="34.0.0"
CMDLINE_TOOLS_URL="https://dl.google.com/android/repository/$CMDLT_FILE"
if [ ! -f $CMDLT_FILE ]; then
    wget $CMDLINE_TOOLS_URL
fi
if [ ! -d ".tmp/cmdline-tools/" ]; then
    mkdir -p ".tmp/cmdline-tools/"
    unzip "$CMDLT_FILE" -d ".tmp/cmdline-tools/"
    mv ".tmp/cmdline-tools/cmdline-tools" ".tmp/cmdline-tools/latest"
fi

SDK_MANAGER=".tmp/cmdline-tools/latest/bin/sdkmanager"
if [ ! -d ".tmp/platform-tools" ]; then
    $SDK_MANAGER "platform-tools"
fi

if [ ! -d ".tmp/platforms/android-$ANDROID_PLATFORM" ]; then
    $SDK_MANAGER "platforms;android-$ANDROID_PLATFORM"
fi

if [ ! -d ".tmp/platforms/android-$ANDROID_PLATFORM" ]; then
    $SDK_MANAGER --install "ndk;$NDK_VERSION"
fi

if [ ! -d ".tmp/build-tools" ]; then
    $SDK_MANAGER --install "build-tools;$BUILD_TOOLS_VERSION"
fi