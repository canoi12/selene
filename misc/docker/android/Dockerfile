FROM openjdk:17-slim

ENV BUILD_DIR=/build
ENV SOURCE_DIR=/source
ENV OUTPUT_DIR=/dist

# Environment variables of Android SDK/NDK
ENV ANDROID_SDK_ROOT=/opt/android-sdk
ENV ANDROID_NDK_VERSION=25.2.9519653
ENV PATH=$ANDROID_SDK_ROOT/cmdline-tools/latest/bin:$ANDROID_SDK_ROOT/platform-tools:$PATH

# Install dependencies
RUN apt-get update && apt-get install -y \
    unzip wget git cmake ninja-build python3 curl \
    && rm -rf /var/lib/apt/lists/*

# Install command-line tools
RUN mkdir -p $ANDROID_SDK_ROOT/cmdline-tools && cd $ANDROID_SDK_ROOT/cmdline-tools && \
    wget https://dl.google.com/android/repository/commandlinetools-linux-9477386_latest.zip -O tools.zip && \
    unzip tools.zip && mv cmdline-tools latest && rm tools.zip

# Accept license and install the SDK
RUN yes | sdkmanager --sdk_root=$ANDROID_SDK_ROOT --licenses
RUN sdkmanager --sdk_root=$ANDROID_SDK_ROOT "platform-tools" "platforms;android-34" "build-tools;34.0.0" "ndk;$ANDROID_NDK_VERSION"

# Install Gradle
RUN wget https://services.gradle.org/distributions/gradle-8.5-bin.zip -O gradle.zip \
    && unzip gradle.zip -d /opt && rm gradle.zip \
    && ln -s /opt/gradle-8.5/bin/gradle /usr/bin/gradle

COPY ./template /android
# COPY ./DownloadSDL2.cmake /DownloadSDL2.cmake
WORKDIR /android

RUN mkdir -p /game && ln -s /game /android/app/src/main/assets

# RUN cmake -DDOWNLOAD_SDL_SOURCE=ON -DCACHE_DIR=/cache -P /DownloadSDL2.cmake
# RUN ls /cache

ENV SELENE_APP_NAME="Selene App"
ENV SELENE_APP_VERSION="0.5.0"
ENV SELENE_APP_NAMESPACE="com.selene.App"
ENV SELENE_ANDROID_ABIS="armeabi-v7a,arm64-v8a"

COPY build.sh /build.sh
RUN chmod +x /build.sh
ENTRYPOINT [ "/build.sh" ]

# CMD cmake -DDOWNLOAD_SDL_SOURCE=ON -DCACHE_DIR=/source/.cache/ -P /DownloadSDL2.cmake && gradle assemble && \