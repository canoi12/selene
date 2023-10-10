# selene

Selene is a tiny game engine made with C and Lua, the idea is to implementing all engine/framework logic in Lua, and for that i expose some C library functions to Lua.

C Modules:
- audio
    - AudioSource
    - AudioBuffers
- SDL2
    - Window
    - GLContext
    - Joystick
    - GameController
- OpenGL
    - Texture
    - Framebuffer
    - Shader
    - Program
    - VertexArray
    - Buffer
- filesystem
    - File
- data
    - Data
- system
- utils (stb_image, stb_truetype, linmath)
    - Mat4

Lua Modules (core):
- class
- audio (Music, Sound)
- graphics (Image, Canvas, Font, Batch, Shader)
- engine (Point, Rect)
- controller
- event
- filesystem
- joystick
- keyboard
- mouse

// main.lua
```lua
local graphics = require 'core.graphics'
local Image = require 'core.graphics.Image

function selene.load()
    image = Image:new('image.png')
end

function selene.update(dt)
end

function selene.draw()
    graphics.fill_rectangle(0, 0, 32, 128)
    graphics.draw(image, nil, 32, 64)
end
```