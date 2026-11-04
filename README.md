<p align="center">
<img src="selene_icon.png" width="256" alt="icon"/>
</p>

# selene

Selene is an executable with embedded Lua modules for game making programming (audio, filesystem, renderer, window, ...).

## Running

After build or download a release file, you can execute the file `selene`, or `selene.exe` on Windows.
Notice that on Windows you will also need to put the `SDL2.dll` in the same folder.

The executable searches for a `main.lua` file, and it uses the lua `require` to do this job, so it will search the main file following the `package.path` hierarchy.
From running a project that is on another directory, you can use the flag `-d` and provide the path (both absolute and relative works), `selene -d examples/renderer`.

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

***Do not forget to call the `selene.set_running(false)` function, or you will get stuck in a infinite loop.***

### Modules

Selene have some bultin modules to easier development, all of them are available as modules:

- audio
- filesystem
- font
- json
- renderer
- window

So to create a window and renderer, you can run:

```lua
local backend = 'opengl'

local opengl = false
local vulkan = false

if backend == 'opengl' then
    opengl = true
    vulkan = false
elseif backend == 'vulkan' then
    opengl = false
    vulkan = true
end
selene{meta={org='Selene',name='App'}}
local win = selene.create_window('Selene App', 640, 380, {opengl=true})
local render = selene.create_renderer(win)

local buffer = ren:create_buffer('vertex', 1024*1024)
local tex, str = ren:load_texture2d('root://selene_icon.png')
if not tex then
    print(tex, str)
end
print(tex)

local batch = renderer.VertexBatch(9*4, 1024)
batch:set_color(1, 1, 1, 1)
batch:set_z(0)
batch:push_vertex2d(-0.5, 0.5, 0, 1, 0, 1, 1, 0, 0)
batch:push_vertex2d(0.5, 0.5, 0, 0, 1, 1, 1, 1, 0)
batch:push_vertex2d(0.5, -0.5, 0, 1, 1, 0, 1, 1, 1)

batch:push_vertex2d(-0.5, 0.5, 0, 1, 0, 1, 1, 0, 0)
batch:push_vertex2d(0.5, -0.5, 0, 1, 1, 0, 1, 1, 1)
batch:push_vertex2d(-0.5, -0.5, 0, 0, 1, 1, 1, 0, 1)

ren:send_buffer_data(buffer, batch:get_offset()*batch:get_stride(), batch:get_data())

-- load vertex shader
local vert_file = 'root://shaders/vert.hlsl'
if backend == 'opengl' then
    vert_file = 'root://shaders/vert.glsl'
    local info = ren:get_info()
    if info.es then
        vert_file = 'root://shaders/vert.glsl100'
    end
elseif backend == 'vulkan' then
    vert_file = 'root://shaders/vert.spv'
end
local vert = ren:load_shader('vertex', vert_file)

-- load fragment shader
local frag_file = 'root://shaders/frag.hlsl'
if backend == 'opengl' then
    frag_file = 'root://shaders/frag.glsl'
    local info = ren:get_info()
    if info.es then
        frag_file = 'root://shaders/frag.glsl100'
    end
elseif backend == 'vulkan' then
    frag_file = 'root://shaders/frag.spv'
end
local frag = ren:load_shader('pixel', frag_file)

print(vert, frag)

-- create render pipeline (WIP)
local pipeline = ren:create_pipeline{
    vs = vert,
    ps = frag,
    layout = {
        stride = 9*4,
        {name = 'a_position', offset = 0, size = 3, type = 'float'},
        {name = 'a_color', offset = 12, size = 4, type = 'float'},
        {name = 'a_texcoord', offset = 28, size = 2, type = 'float'},
    },
    blend = {enabled = true, func = 'alpha'},
    scissor = {enabled = true},
    descriptors = {
        {type = 'combined_image_sampler', stage = 'fragment'}
    }
}

selene.set_event(function(name, ...)
    if name == 'quit' or name == 'window closed' then selene.set_running(false) end
end)
selene.set_step(function()
    ren:set_render_target()
    ren:set_viewport(0, 0, 640, 380)
    ren:set_scissor(0, 0, 640, 380)
    ren:clear_color(0.3, 0.4, 0.4, 1.0)
    ren:clear_depth(1)
    ren:set_pipeline(pipeline)
    ren:set_vertex_buffer(buffer)
    ren:set_texture(tex)
    ren:draw('triangles', 0, 6)
    ren:present()
    selene.delay(16)
end)

selene.set_quit(function()
    batch:destroy()
    if pipeline then ren:destroy_pipeline(pipeline) end
    if tex then ren:destroy_texture(tex) end
    ren:destroy_shader(vert)
    ren:destroy_shader(frag)
    ren:destroy_buffer(buffer)
    ren:destroy()
    win:destroy()
end)
```

And you can use the plugins (WIP) to abstract the creation of pipelines and other technical stuff:

```lua
local RenderBatch2D = require('plugins.RenderBatch2D')
print(RenderBatch2D, RenderBatch2D.create)

selene({ meta = {org = 'My Company', name = 'Roguelike'}})
local win = selene.create_window('Roguelike', 640, 380, {opengl = true})
local ren = RenderBatch2D.create(win)

selene.set_event(function(name)
    if name == 'quit' or name == 'window closed' then selene.set_running(false) end
end)

selene.set_step(function()
    ren:begin_frame()
    ren:clear(0.3, 0.4, 0.4, 1.0)
    -- ren:set_color(1, 1, 1, 1)
    ren:draw_circle(128, 96, 32)
    ren:draw_text(ren.font, 'olar')
    ren:end_frame()
end)

selene.set_quit(function()
    ren:destroy()
    win:destroy()
end)
```

## Building

First you need to install [CMake](https://cmake.org/) on your system.

I put the [premake5](https://premake.github.io/) scripts on the `misc/scripts/` folder, but I don't have plans to update it by now.

### Presets (recommended)

The repository ships a `CMakePresets.json` with per-platform presets. Each
preset configures into its own `build/<preset>` directory, so different
configurations never share a stale cache.

```bash
cmake --list-presets
cmake --preset linux-debug
cmake --build --preset linux-debug
ctest --preset linux-debug
```

| Preset | Platform | Notes |
|--------|----------|-------|
| `linux-debug` / `linux-release` | Linux | Ninja; uses the system SDL2 when available |
| `linux-sdl-source` | Linux | Forces SDL2 from `.cache/SDL2-2.32.4/source` |
| `windows-msvc-x64` / `windows-msvc-win32` | Windows | Visual Studio 2022 |
| `mingw-x64` | Linux → Windows | MinGW cross toolchain |
| `emscripten` | Web | Requires `EMSDK` |
| `android-arm64` | Android | Requires `ANDROID_HOME` |

The SDL2 paths used by the presets are repo-relative; they can always be
overridden from the command line, for example
`cmake --preset linux-debug -DSDL_DIR=/opt/sdl2`. Machine specific paths can
live in a `CMakeUserPresets.json` (git-ignored) that inherits from the shipped
presets.

### SDL2

SDL2 is located by `cmake/FindCustomSDL2.cmake`, in this order:

1. `-DSDL2_DIR=<dir containing SDL2Config.cmake>` (highest priority);
2. `-DSDL_DIR=<path>` — the root of a precompiled package (MSVC/MinGW devel
   archives), an installation prefix, **or** the SDL2 source tree;
3. the system SDL2 (`libsdl2-dev`, Homebrew, ...);
4. building SDL2 from source (`-DUSE_SDL_SOURCE=ON` plus a source tree).

When `SDL_DIR` is not provided it defaults, per platform, to
`.cache/SDL2-2.32.4/{MSVC,MINGW,source}`. Keep that version in sync with
`SDL_VERSION` in `cmake/Config.cmake` and with `CMakePresets.json`.

`SDL2_DIR` always wins over `SDL_DIR`. Because it is a normal CMake cache
entry, changing `SDL_DIR` on an already configured build directory may need
`cmake --preset <preset> --fresh` (or simply use another preset, since each one
has its own build directory).

`SDL_PRECOMP_DIR` is deprecated: it is still accepted as an alias of `SDL_DIR`
and prints a deprecation warning.

To download the SDL2 packages use:

```bash
cmake -P cmake/scripts/DownloadSDL2.cmake
# force a specific package:
cmake -P cmake/scripts/DownloadSDL2.cmake -DDOWNLOAD_SDL_MINGW=ON
```

### Linux

You will need to install the `SDL2` packages for development too, the package is present in the majority of the Linux distros.

In Debian you can install SDL2 with:

```
sudo apt install libsdl2-dev
```

And then use the presets:

```bash
cmake --preset linux-debug
cmake --build --preset linux-debug
```

Without presets the equivalent is:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=[Debug|Release]
cmake --build build
```

It will generate the build files in the `build/` folder.

### Windows

For Windows, you will also need to download the [SDL2](https://libsdl.org/)
package for development (or let `cmake/scripts/DownloadSDL2.cmake` place it in
`.cache/SDL2-2.32.4/MSVC`), and run:

```bash
cmake --preset windows-msvc-x64
cmake --build --preset windows-msvc-x64-release
```

Without presets: `cmake -B build -G "Visual Studio 17 2022" -A x64 -DSDL_DIR=[Path to SDL2 MSVC devel package]`

### Cross Compiling

On linux you can use the CMake Toolchains to cross compile between architectures.

```
cmake -B build -DCMAKE_TOOLCHAIN_FILE=[toolchain file]
cmake --build build
```

Or use a preset, for example `cmake --preset mingw-x64` (see `CMakePresets.json`).

#### Toolchains

You can find the toolchains in the `cmake/toolchains` folder, and you can edit them or create new ones as you need. To cross compile for MinGW, for example:

```
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/MinGW.cmake -DSDL_DIR=[Path to SDL2 MinGW dev package]
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
