# Selene

Selene is a tiny game engine made with C and Lua, the idea is to implementing all engine/framework logic in Lua, and for that i expose some C library functions to Lua.

## Modules

`C Modules:`
- Data
- audio (drwav, stb_vorbis)
    - Decoder
- filesystem
    - File
- font (stb_truetype, default bit font)
- image (stb_image)
- linmath
    - Mat4
- OpenGL
    - Texture
    - Framebuffer
    - Shader
    - Program
    - VertexArray
    - Buffer
- SDL2
    - AudioDeviceID
    - AudioStream
    - Event
    - Gamepad (GameController)
    - Joystick
    - GLContext
    - Window
- system

`Lua Modules (core):`
- class
- audio
    - Music
    - Sound
    - SoundInstance
    - AudioSystem
- graphics
    - Image
    - Canvas
    - Font
    - Batch
    - Shader
    - Renderer
- input
    - Gamepad
    - Joystick
    - Keyboard
    - Mouse
    - InputSystem
- App
- Event
- Filesystem
- Point
- Rect
- Settings
- Window

## Running projects

Just execute selene passing your project path as argument, if none is provided the current directory will be used.

The directory must contain a `main.lua` file and a `.selene/` folder

`usage: ./selene [path/to/project]`

The idea is that by using:

```lua
local App = require('App')
return App.default()
```

The project will use the selene default engine.

But you can write your own loop by using

### main.lua
```lua
local App = require('App')
local Color = require('graphics.Color')
local Settings = require('Settings')

local s = Settings.create("game", 640, 380)
local app = App.create(s)

function app:update(dt)
end

function app:draw(render)
    render:begin()
    render:clearColor(Color.black)
    render:clear()

    render:fillRectangle(32, 32, 64, 128)
    render:finish()
end
return app
```

### core/

All the Lua modules are located in a directory called `core/`, and it must be in the same location of the Selene executable.
On startup Selene will search for this file, and if not found it will not boot up.

This is the bootup script:
```lua
local sdl = selene.sdl2
local function add_path(path)
    package.path = path .. '?.lua;' .. path .. '?/init.lua;' .. package.path
end
return function(args)
    add_path(sdl.getBasePath())
    return require('core')
end
```

## Dependencies

- A C compiler
- SDL2 dev lib
- CMake

### Linux

On Linux i use Make for build, so in Debian for example you can install all dependencies with:

```
sudo apt install gcc make libsdl2-dev cmake
```

and build with:

```
cmake -B build && make -C build
```


### Windows

If you going to use the VC++ compiler and Visual Studio Tools, you will need to download the SDL2 dev lib from https://github.com/libsdl-org/SDL/releases/ (Download the SDL2-devel-x.xx.x-VC.zip version), and put the content in external/SDL2/MSVC.


### MacOS

For OSX i recommend use [homebrew](https://brew.sh/), you can install all dependencies with:

```
brew install sdl2 cmake make 
```

(The C compiler is already embedded when you install the build tools)

And for build is the same as Linux:
```
cmake -B build && make -C build
```
