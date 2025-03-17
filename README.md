<p align="center">
<img src="selene_icon.png" width="256" alt="icon"/>
</p>

# selene

Selene is an executable with embedded Lua modules for game making programming (sdl2, opengl, linmath, json, asset loaders).

## Running

After build or download a release file, you can execute the file `selene`, or `selene.exe` on Windows.
Notice that on Windows you will also need to put the `SDL2.dll` in the same folder.

The executable searches for a `main.lua` file, and it uses the lua `require` to do this job, so it will search the main file following the `package.path` hierarchy.
From running a project that is on another directory, you can use the flag `-d` and provide the path (both absolute and relative works), `selene -d examples/sdl2`.

### main.lua

As I said, the entry file is a `main.lua`. The selene structure use two simple callbacks, `step` in the main loop and `quit` when closing. You can set them using `selene.set_step(function)` or `selene.set_quit(function)`.

```lua
-- main.lua
local i = 1
selene.set_step(function()
    print(i, 'aloooooooooooooooooooooo')
    i = i + 1
    if i > 50 then
        selene.set_running(false) -- do not forget to call this function to exit, otherwise the application will continue running the main loop
    end
end)
selene.set_quit(function()
    print('exiting')
end)
```

### SDL2 example

```lua
-- main.lua
if not sdl.init(sdl.INIT_VIDEO) then
    error('Failed to init SDL2 ' .. sdl.get_error())
end
local win = sdl.create_window('SDL2 example', sdl.WINDOWPOS_CENTERED, sdl.WINDOWPOS_CENTERED, 640, 380, sdl.WINDOW_SHOWN)
if not win then
    error("Failed to create SDL2 window " .. sdl.get_error())
end
local renderer = sdl.create_renderer(win)
local event = sdl.create_event()
selene.set_step(function()
    while event:poll() do
        local t = event:get_type()
        if t == sdl.QUIT then selene.set_running(false) end
        if t == sdl.WINDOWEVENT then
            if t == sdl.WINDOWEVENT_CLOSE then selene.set_running(false) end
        end
    end
    renderer:set_color(75, 125, 125, 255)
    renderer:clear()
    renderer:present()
end)
selene.set_quit(function()
    renderer:destroy()
    win:destroy()
    sdl.quit()
end)
```

### Selene runner (WIP)

The selene runner will be the easier way to start your application, it will be a C core with an bunch of integrated systems
like OpenGL renderer, audio system and filesystem, fully powered by SDL2 and SDL3.

```lua
selene("MyGame", "1.0.0", "org.selene.MyGame")
local render = selene.renderer.RenderBatch2D(selene.get_renderer())
selene.set_event(function(name, ...)
    if name == 'quit' or name == 'window closed' then selene.set_running(false) end
end)
selene.set_step(function()
    render:begin()
    render:enable_3d()
    render:clear(0.2, 0.3, 0.3)
    render:draw_cube({ 32, 64, -32 }, { math.rad(15), math.rad(60), 0 }, { 32, 32, 32 })
    render:disable_3d()
    render:finish()
    render:present()
end)
```

## Building

First you need to install [CMake](https://cmake.org/) on your system.

I put the [premake5](https://premake.github.io/) scripts on the `misc/scripts/` folder, but I don't have plans to update it by now.

### SDL2

By default the CMake script checks for SDL dists in the `.cache/SDL2` folder.

- MINGW
- MSVC
- source (for source code)

But you can specify the paths with:

- -DSDL_PRECOMP_DIR=\[Precompiled dir\] (SDL provides that for MINGW and MSVC)
- -DSDL_SOURCE_DIR=\[SDL source directory\]

You can also download SDL2 using the script inside `scripts/download_sdl.sh`.

### Linux

You will need to install the `SDL2` packages for development too, the package is present in the majority of the Linux distros.

In Debian you can install SDL2 with:

```
sudo apt install libsdl2-dev
```

And then run CMake:

```
cmake -B build -DCMAKE_BUILD_TYPE=[Debug|Release]
```

It will generate the build files in the `build/` folder, then just run:

```
cmake --build build
```

### Windows

For Windows, you will also need to download the [SDL2](https://libsdl.org/) package for development. Making that, simply run on a terminal:

```
cmake -B build -DSDL_PRECOMP_DIR=[Path to SDL2 MSVC dev dist, callbacks to .cache/SDL2/MSVC]
```

### Cross Compiling

On linux you can use the CMake Toolchains to cross compile between architectures.

```
cmake -B build -DCMAKE_TOOLCHAIN_FILE=[toolchain file]
cmake --build build
```

#### Toolchains

You can find the toolchains in the `cmake/toolchains` folder, and you can edit them or create new ones as you need. To cross compile for MinGW, for example:

```
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/MinGW.cmake -DSDL_PRECOMP_DIR=[Path to SDL2 MinGW dev dist]
cmake --build build
```

And the process is very similar to Emscripten and Android builds, you can use the toolchains provided by those SDKs.

#### Emscripten

Download and setup the Emscripten SDK and run:

```
cmake -B build -DCMAKE_TOOLCHAIN_FILE=${EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake
cmake --build build
```

To a fast emsdk setup run `scripts/setup.sh emscripten`, it will download the SDK to the `.cache/` folder.

#### Android

Download and setup the Android SDK and NDK, and run the CMake with the Android toolchain:

```
cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=${ANDROID_HOME}/ndk/${NDK_VERSION}/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=[armeabi-v7a, arm64-v8a, x86, x86_64] \
    -DANDROID_PLATFORM=android-*[21, 22, ...]
cmake --build build
```

To a fast Android SDK setup, use `scripts/setup.sh android`, it will download all needed files to `.cache/`.
In the future I plan to add arg options to control SDK, NDK and platform versions.
