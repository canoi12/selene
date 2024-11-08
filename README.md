# selene

An executable with embedded Lua modules for game making programming (or any other application, graphical or not).

## build

First you need to install [premake5](https://premake.github.io/) on your system.

### Windows

For Windows, you will also need to download the [SDL2](https://libsdl.org/) package for development. Making that, simply run:

```
premake5 vs2019 --sdl-dir={source to SDL folder}
```

It will generate the Visual Studio project files in the `build/` folder. After that, you can open the `build\selene.sln` solution in Visual Studio, or start `Developer Command Prompt for VS20**` and run:

```
msbuild build\selene.sln -p:Configuration=[Release|Debug];Platform=[win32|win64]
```

### Linux

For linux, the process is very similar to Windows. You will need to install the `SDL2` packages for development too, but it is still present in the majority of the Linux distros.

In Debian you can install SDL2 with:

```
sudo apt install libsdl2-dev
```

And then run premake:

```
premake5 gmake2
```

It will generate Makefiles in the `build/` folder, then just run:

```
make -C build/ config=[debug_linux,release_linux]
```

*Build for MinGW is a very similar process*

### Other platforms

I made some tests of build for `Android` and `Emscripten` as well, later I will update this README with the steps to build for those platforms.