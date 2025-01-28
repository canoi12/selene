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

## Emscripten

The emscripten generated selene.html searches for a `game.js` file, you can generate one based on your project with:

```
${EMSDK}/upstream/emscripten/tools/file_packager game --embed [your game path]@./ >> game.js
```