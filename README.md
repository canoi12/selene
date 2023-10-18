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
- graphics
    - Image
    - Canvas
    - Font
    - Batch
    - Shader
- engine
    - Point
    - Rect
- gamepad
- event
- filesystem
- joystick
- keyboard
- mouse

## Running projects

Just execute selene passing your project path as argument, if none is provided the current directory will be used.

The directory must contain a `main.lua` file

`usage: ./selene [path/to/project]`


### main.lua
```lua
local graphics = require 'core.graphics'
local Image = require 'core.graphics.Image'

function selene.load()
    image = Image('image.png')
end

function selene.update(dt)
end

function selene.draw()
    graphics.fill_rectangle(0, 0, 32, 128)
    graphics.draw(image, nil, 32, 64)
end
```

### core/

All the Lua modules are located in a directory called `core/`, and it must be in the same location of the Selene executable.
On startup, it will find for this module, and if not found, Selene will not boot up.

This is the bootup script:
```lua
local sdl = selene.sdl2
local function add_path(path)
    package.path = path .. '?.lua;' .. path .. '?/init.lua;' .. package.path
end
return function(args)
    add_path(sdl.GetBasepath())
    local core = require 'core'
    core.init(selene.args)
    return core
end
```
