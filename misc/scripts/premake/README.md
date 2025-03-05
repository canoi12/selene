# Building with premake

### Windows

For Windows, you will also need to download the [SDL2](https://libsdl.org/) package for development. Making that, simply run on a terminal:

```
premake5 vs2019 --sdl-dir={source to SDL folder}
```

It will generate the Visual Studio project files in the `build/` folder. After that, you can open the `build\selene.sln` solution in Visual Studio, or start the `Developer Command Prompt for VS20**` and run:

```
msbuild build\selene.sln -p:Configuration=[Release|Debug];Platform=[win32|win64]
```

### Linux

For linux the process is very similar. You will need to install the `SDL2` packages for development too, the package is present in the majority of the Linux distros.

In Debian you can install SDL2 with:

```
sudo apt install libsdl2-dev
```

And then run premake:

```
premake5 gmake2
```

It will generate the Makefiles in the `build/` folder, then just run:

```
make -C build/ config=[debug_linux,release_linux]
```

*Building for MinGW is a very similar process*

### Emscripten

You need the module [premake-emscripten](https://github.com/tritao/premake-emscripten) for premake5. Put in the same directory of the premake5 binary and rename the module folder to `emscripten`. After that, execute this in the selene source directory:

`premake5 gmake2 --emscripten`

And run the Makefile with:

`make -C build config=[release_emscripten|debug_emscripten]`

Notice that you will need to setup the EMSDK too, https://emscripten.org/.

#### Packaging for Emscripten

In the release distribution of selene for Emscripten, the index.html searches for a `game.js` file.
It can be generated using the file packaging tools of the EMSDK itself. I create a script for premake5 called `gen_gamejs.lua` that can help with that.
One you have the EMSDK variable properly setted on your system, it should work. You can use like this:

`premake5 build --file=gen_gamejs.lua --project-dir=[path to your project] --out-dir=build/bin/emscripten/Release/`

### Android

TODO