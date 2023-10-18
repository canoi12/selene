# selene

Selene is a tiny game engine made with C and Lua, the idea is to implementing all engine/framework logic in Lua, and for that i expose some C library functions to Lua.

C Modules:
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

Lua Modules (core):
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

`usage: ./selene path/to/project`

// main.lua
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