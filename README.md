<p align="center">
<img src="selene_icon.svg" width="256" alt="icon"/>
</p>

# selene

An executable with embedded Lua modules for game making programming (sdl2, opengl, linmath, json, asset loaders).

## Building

First you need to install [premake5](https://premake.github.io/) on your system.

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

### Other platforms

I made some tests building for `Android` and `Emscripten` as well, later I will update this README with the steps to build for those platforms.

## Running

After build or download a release file, you can execute the binay: `selene` or `selene.exe` on Windows.
Notice that on Windows you will also need to put the SDL2.dll on the same folder.

The executable searches for a main.lua file, and it uses the lua `require` to do this job, so it will search the main file following the `package.path` hierarchy.
From running a project that is on another directory, you can use the flag `-d` and provide the path (both absolute and relative works), `selene -d examples/sdl2`.
