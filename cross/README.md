# Cross compiling templates

## Android

Get the `cross/android/template` and throw in any folder inside the project root path, I'm using `build/` as example.

Enter in the `build/` folder and run `./gradlew` (or `./gradlew.exe` on Windows). You can edit the build settings in the `build/app/build.graple`, even the ndkVersion in `android.ndkVersion` key.

```
cp -r cross/android/template build/
cd build/
./gradlew build
```

To embed a game in the .apk, you can copy the content of your game and put on `build/app/src/main/assets` folder.

### Setup Android SDK and NDK

Simple guide to setup Android SDK and NDK using Linux. Before beginning, you need to install de Java JDK, on Debian I'm using: `sudo apt install openjdk-17-*`

Download the command line tools for Linux from https://developer.android.com/studio, you can either download Android Studio or download the command line tools alone.

Once you download the tools, put it in that structure: `android_sdk/cmdline-tools/latest/[content of command line tools]`. `android_sdk` can have any name, but the other folders must follow the structure. And then run:

```
android_sdk/cmdline-tools/latest/bin/sdkmanager "platform-tools" "plaforms;android-21"
```

And to install NDK:

```
android_sdk/cmdline-tools/latest/bin/sdkmanager --install "ndk;{your ndk version}"
```

To see the other packages run `android_sdk/cmdline-tools/latest/bin/sdkmanager --list` and consult https://developer.android.com/tools/sdkmanager.

## Emscripten

The emscripten generated selene.html searches for a `game.js` file, you can generate one based on your project with:

```
${EMSDK}/upstream/emscripten/tools/file_packager game --embed [your game path]@./ >> game.js
```

More informations on https://emscripten.org/docs/porting/files/packaging_files.html
