local plugins, state = require('plugins')
print(plugins, state)
plugins.setup()
print('gl lib', gl)
print(os.host(), os.arch())
print(os.clock(), os.tmpname())
print(selene.get_version())
local ldtk = require('ldtk')
local graphics = require('graphics')
local Image = require('graphics.Image')
local Effect = require('graphics.Effect')

print(ldtk)
print(utf8, utf8.codepoint('o'))
print(selene.__dir)

sdl.init(sdl.INIT_VIDEO)
print('garaio')
local g = graphics.create({ title = 'game' })
local ev = sdl.create_event()

local project = ldtk.load('game.ldtk')
local level = project.levels[1]

local vertices = selene.create_data(6 * 8 * 4)
vertices:write_floats(0,
    0, 0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0,
    256, 0, 1.0, 1.0, 0.0, 1.0, 1.0, 0.0,
    256, 256, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0,
    0, 0, 1.0, 0.0, 1.0, 1.0, 0.0, 0.0,
    256, 256, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0,
    0, 256, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0
)

local ArrayBuffer = require('gl_helper.ArrayBuffer')
local VertexArray = require('gl_helper.VertexArray')

-- vertices:writeFloats(6, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5)
local buff = ArrayBuffer.create()
local vao = VertexArray.create()

print(vertices, vertices.size, vertices.root)


vao:bind()
buff:bind()
buff:data(vertices, 'STATIC_DRAW')
-- gl.buffer_data(gl.ARRAY_BUFFER, vertices.size, vertices.root, gl.STATIC_DRAW)
-- graphics.gl.Buffer.data(gl.ARRAY_BUFFER, gl.STATIC_DRAW, vertices.size, vertices.root)
gl.enable_vertex_attrib_array(0)
gl.enable_vertex_attrib_array(1)
gl.enable_vertex_attrib_array(2)
gl.vertex_attrib_pointer(0, 2, gl.FLOAT, false, 8 * 4, 0)
gl.vertex_attrib_pointer(1, 4, gl.FLOAT, false, 8 * 4, 2*4)
gl.vertex_attrib_pointer(2, 2, gl.FLOAT, false, 8 * 4, 6*4)
vao:unbind()
buff:unbind()

local effect = Effect.create()
print(effect, effect.program, effect.vertShader)
local mat4 = linmath.mat4x4
local persp = linmath.mat4x4.alloc()
mat4.identity(persp)
mat4.ortho(persp, 0, 640, 380, 0, -1, 1)

local view = linmath.mat4x4.alloc()
mat4.identity(view)

gl.use_program(effect.program)
effect:send_matrix("u_MVP", persp)
effect:send_matrix("u_View", view)
gl.use_program()

local img = project.tilesets[1].texture

selene.set_step(
    function()
        while ev:poll() do
            if ev:get_type() == sdl.QUIT then selene.set_running(false) end
        end

        for i,layer in ipairs(level.layers) do
            if layer.identifier == 'AutoLayer' then
                for j,tile in ipairs(layer.tiles) do
                    
                end
            end
        end

        gl.clear_color(0.3, 0.4, 0.4, 1)
        gl.clear(gl.COLOR_BUFFER_BIT)

        gl.bind_texture(gl.TEXTURE_2D, img.handle)
        gl.use_program(effect.program)
        vao:bind()
        -- gl.bind_buffer(gl.ARRAY_BUFFER, buff)
        gl.draw_arrays(gl.TRIANGLES, 0, 6)
        -- gl.bind_buffer(gl.ARRAY_BUFFER)
        gl.bind_texture(gl.TEXTURE_2D)
        vao:unbind()
        gl.use_program()
        g:swap()
    end
)

selene.set_quit(
    function ()
        g:destroy()
        sdl.quit()
    end
)
