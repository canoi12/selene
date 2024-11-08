require('plugins').setup()
local sdl = require('selSDL')
sdl.init(sdl.INIT_EVERYTHING)
sdl.glSetAttribute(sdl.GL_CONTEXT_MAJOR_VERSION, 3)
sdl.glSetAttribute(sdl.GL_CONTEXT_MINOR_VERSION, 2)
sdl.glSetAttribute(sdl.GL_CONTEXT_PROFILE_MASK, sdl.GL_CONTEXT_PROFILE_CORE)
local window = sdl.Window.create("SDL2 Test", sdl.WINDOWPOS_CENTERED, sdl.WINDOWPOS_CENTERED, 640, 380, {sdl.WINDOW_SHOWN, sdl.WINDOW_OPENGL})
-- local render = sdl.Renderer.create(window)
local ctx = sdl.GLContext.create(window)
gl.setup()
local event = sdl.Event.create()

local current = sdl.getTicks()
local last = sdl.getTicks()

function createOrtho(left, right, bottom, top, near, far)
    local m = {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
    }
    m[1] = 2 / (right - left)
    m[6] = 2 / (top - bottom)
    m[11] = -2 / (far - near)
    m[13] = -(right + left) / (right - left)
    m[14] = -(top + bottom) / (top - bottom)
    m[15] = -(far + near) / (far - near)
    m[16] = 1
    return m
end

local buf = gl.gen_buffers(1)
local vert = gl.create_shader(gl.VERTEX_SHADER)
local frag = gl.create_shader(gl.FRAGMENT_SHADER)

vert:source([[
#version 120
attribute vec2 position;
attribute vec3 color;
attribute vec2 texcoord;

uniform mat4 world;

varying vec3 v_color;
varying vec2 v_texcoord;

void main() {
    gl_Position = world * vec4(position, 0, 1);
    v_color = color;
    v_texcoord = texcoord;
    }
]])
frag:source([[
#version 120
varying vec3 v_color;
varying vec2 v_texcoord;

uniform sampler2D u_tex;

void main() {
    gl_FragColor = texture2D(u_tex, v_texcoord) * vec4(v_color, 1);
}
]])
vert:compile()
frag:compile()

local prog = gl.create_program()
prog:attachShader(vert, frag)
prog:link()

gl.use_program(prog)
local loc = prog:getUniformLocation('world')
print(loc)
gl.uniformMatrix4fv(loc, 1, false, createOrtho(0, 640, 380, 0, -1, 1))
-- gl.uniformMatrix4fv(loc, 1, false, {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1})
gl.use_program()


vert:destroy()
frag:destroy()
-- print(selene.fs, selene.image, selene.data)
-- for i,v in pairs(selene) do
--     print(i, v)
-- end
local xx = 0
local data = selene.Data.create(4 * 7 * 3)
data:writeFloats(0, 320, 0, 1, 0, 1, 0.5, 0)
data:writeFloats(28, 640, 380, 0, 1, 1, 1, 1)
data:writeFloats(56, 0, 380, 1, 1, 0, 0, 1)

-- data:writeFloats(0, 0, 0.5, 1, 0, 1, 0.5, 0)
-- data:writeFloats(28, 0.5, -0.5, 0, 1, 1, 0, 1)
-- data:writeFloats(56, -0.5, -0.5, 1, 1, 0, 1, 1)

local vao = gl.VertexArray.create()

vao:bind()
gl.Buffer.bind(gl.ARRAY_BUFFER, buf)
gl.Buffer.data(gl.ARRAY_BUFFER, gl.STATIC_DRAW, data:getSize(), data)

gl.VertexArray.enable(0)
gl.VertexArray.enable(1)
gl.VertexArray.enable(2)
gl.VertexArray.attribPointer(0, 2, gl.FLOAT, gl.FALSE, 7*4, 0)
gl.VertexArray.attribPointer(1, 3, gl.FLOAT, gl.FALSE, 7*4, 2*4)
gl.VertexArray.attribPointer(2, 2, gl.FLOAT, gl.FALSE, 7*4, 5*4)

gl.Buffer.bind(gl.ARRAY_BUFFER)
vao.bind()

local img_data = {selene.image.load('tuiteiro.jpg')}
print(img_data[1], img_data[2], img_data[3])

local tex = gl.Texture.create()
gl.Texture.bind(gl.TEXTURE_2D, tex)
gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
-- gl.Texture.paremeteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
gl.Texture.image2D(gl.TEXTURE_2D, gl.RGBA, img_data[2], img_data[3], gl.RGBA, gl.UNSIGNED_BYTE, img_data[1])
gl.Texture.bind(gl.TEXTURE_2D)

selene.setStep(
function()
    while event:poll() do
        if event:getType() == sdl.QUIT then selene.setRunning(false) end
        if event:getType() == sdl.WINDOWEVENT then
            if event:windowEvent() == sdl.WINDOWEVENT_CLOSE then selene.setRunning(false) end
        end
    end
    last = current
    current = sdl.getTicks()
    local delta = (current - last) / 1000
    gl.clearColor(0.3, 0.3, 0.4, 1.0)
    gl.clear(gl.COLOR_BUFFER_BIT)

    gl.Texture.bind(gl.TEXTURE_2D, tex)
    gl.Program.use(prog)
    vao:bind()
    gl.drawArrays(gl.TRIANGLES, 0, 3)
    vao.bind()
    gl.Program.use()
    gl.Texture.bind(gl.TEXTURE_2D)

    -- render:setColor(75, 75, 125, 255)
    -- render:clear()
    -- render:present()
    window:glSwap()
end
)


selene.setQuit(
function()
    tex:destroy()
    buf:destroy()
    prog:destroy()
    vao:destroy()

    -- render:destroy()
    ctx:destroy()
    window:destroy()
    sdl.quit()
end
)
