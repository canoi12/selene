require('plugins').setup()
local model_loader = require('model_loader')
sdl.init(sdl.INIT_EVERYTHING)
sdl.gl_set_attribute(sdl.GL_CONTEXT_MAJOR_VERSION, 3)
sdl.gl_set_attribute(sdl.GL_CONTEXT_MINOR_VERSION, 2)
sdl.gl_set_attribute(sdl.GL_CONTEXT_PROFILE_MASK, sdl.GL_CONTEXT_PROFILE_CORE)
local window = sdl.create_window("3D Example", sdl.WINDOWPOS_CENTERED, sdl.WINDOWPOS_CENTERED, 640, 380, sdl.WINDOW_SHOWN | sdl.WINDOW_OPENGL)
-- local render = sdl.Renderer.create(window)
local ctx = sdl.create_gl_context(window)
gl.setup()
local event = sdl.create_event()
local keys = sdl.get_keyboard_state()
print(selene.__dir)

print(gl.get_string(gl.VERSION))

local current = sdl.get_ticks()
local last = sdl.get_ticks()

local vert = gl.create_shader(gl.VERTEX_SHADER)
local frag = gl.create_shader(gl.FRAGMENT_SHADER)

gl.shader_source(vert, [[
#version 100
attribute vec3 position;
attribute vec3 normal;
attribute vec2 texcoord;

uniform mat4 world;
uniform mat4 view;
uniform mat4 model;

varying vec3 v_color;
varying vec2 v_texcoord;

void main() {
    gl_Position = world * view * model * vec4(position, 1);
    v_color = normal;
    v_texcoord = texcoord;
}
]])
gl.shader_source(frag, [[
#version 100
precision mediump float;
varying vec3 v_color;
varying vec2 v_texcoord;

uniform sampler2D u_tex;

void main() {
    gl_FragColor = /* texture2D(u_tex, v_texcoord) * */ vec4(1, 0, 0, 1);
}
]])
gl.compile_shader(vert)
gl.compile_shader(frag)

local prog = gl.create_program()
gl.attach_shader(prog, vert)
gl.attach_shader(prog, frag)
gl.link_program(prog)

gl.use_program(prog)

-- Set world matrix
local loc = gl.get_uniform_location(prog, 'world')
local mat4 = linmath.mat4x4
local mat = mat4.alloc()
mat4.identity(mat)
mat4.perspective(mat, math.rad(45), 640/380, 0.1, 1000)
gl.uniform_matrix4fv(loc, 1, false, mat)

-- Set view matrix
loc = gl.get_uniform_location(prog, 'view')
mat4.identity(mat)
mat4.translate(mat, 0, 0, -3)
gl.uniform_matrix4fv(loc, 1, false, mat)

-- Set model matrix
loc = gl.get_uniform_location(prog, 'model')
mat4.identity(mat)
gl.uniform_matrix4fv(loc, 1, false, mat)

gl.use_program()

gl.delete_shader(vert)
gl.delete_shader(frag)

local catModel = model_loader.Model.load(selene.__dir .. '/CatModel/12221_Cat_v1_l3.obj')
print(catModel)

-- local data = selene.Data.create(10000000)
-- local indices_data = selene.Data.create(6000000)

-- local cube_data = require('CatModel')

-- for i,vert in ipairs(cube_data.vertices) do
    -- print(i, table.unpack(vert))
    -- data:writeFloats((i-1)*32, table.unpack(vert))
-- end
-- indices_data:writeInts(0, table.unpack(cube_data.indices))

local vao = gl.gen_vertex_arrays(1)

local buf = gl.gen_buffers(1)
local index = gl.gen_buffers(1)

gl.bind_vertex_array(vao)
gl.bind_buffer(gl.ARRAY_BUFFER, buf)
gl.bind_buffer(gl.ELEMENT_ARRAY_BUFFER, index)
gl.buffer_data(gl.ARRAY_BUFFER, catModel.num_vertices * 5 * 4, catModel.vertices, gl.STATIC_DRAW)
gl.buffer_data(gl.ELEMENT_ARRAY_BUFFER, catModel.num_indices * 4, catModel.indices, gl.STATIC_DRAW)
print(gl.get_attrib_location(prog, 'normal'), gl.get_attrib_location(prog, 'texcoord'))
print(catModel.vertices, catModel.indices)

gl.enable_vertex_attrib_array(0)
-- gl.enable_vertex_attrib_array(1)
gl.enable_vertex_attrib_array(2)
gl.vertex_attrib_pointer(0, 3, gl.FLOAT, false, 0, 0)
-- gl.vertex_attrib_pointer(1, 3, gl.FLOAT, gl.FALSE, 0, catModel.num_vertices * 3 * 4)
gl.vertex_attrib_pointer(2, 2, gl.FLOAT, false, 0, catModel.num_vertices * 3 * 4 * 2)

gl.bind_vertex_array()
gl.bind_buffer(gl.ARRAY_BUFFER)
gl.bind_buffer(gl.ELEMENT_ARRAY_BUFFER)

local img_data = selene.ImageData.load(selene.__dir .. '/CatModel/Cat_diffuse.jpg')
print(img_data, img_data.width, img_data.height, img_data.pixels)

local tex = gl.gen_textures(1)
gl.bind_texture(gl.TEXTURE_2D, tex)
gl.tex_parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
gl.tex_parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
gl.tex_parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
-- gl.Texture.paremeteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
gl.tex_image2d(gl.TEXTURE_2D, 0, gl.RGBA, img_data.width, img_data.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, img_data.pixels)
gl.bind_texture(gl.TEXTURE_2D)

gl.enable(gl.DEPTH_TEST)
gl.enable(gl.BLEND)
gl.blend_func(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA)
gl.blend_equation(gl.FUNC_ADD)
local time = 0

local default_delta = 1 / 60.0
local FPS = 60.0
local DELAY = 1000.0 / FPS

local positions = {
    {0, -5, 0},
    -- {2, 3, -3},
    -- {5, -2, -3},
    -- {-2, 6, -15},
    -- {-2, 6, 8},
    -- {-4, 4, 4},
}

local mult = 1

selene.set_step(
function()
    while event:poll() do
        if event:get_type() == sdl.QUIT then selene.set_running(false) end
        if event:get_type() == sdl.WINDOWEVENT then
            if event:window_event() == sdl.WINDOWEVENT_CLOSE then selene.set_running(false) end
        end
        if event:get_type() == sdl.KEYDOWN then
            local code = event:keyboard_event()
            if code == sdl.get_scancode_from_name('left') then mult = mult - 1
            elseif code == sdl.get_scancode_from_name('right') then mult = mult + 1 end
        end
    end
    last = current
    current = sdl.get_ticks()
    local delta = (current - last)
    local deltaTime = delta / 1000
    time = time + deltaTime

    while deltaTime > 0.0 do
        local dt = math.min(delta, default_delta)
        -- if self.onUpdate then self:onUpdate(dt) end
        deltaTime = deltaTime - dt
    end

    gl.clear_color(0.3, 0.3, 0.4, 1.0)
    gl.clear(gl.COLOR_BUFFER_BIT, gl.DEPTH_BUFFER_BIT)
    
    gl.bind_texture(gl.TEXTURE_2D, tex)
    gl.use_program(prog)
    gl.bind_buffer(gl.ELEMENT_ARRAY_BUFFER, index)
    
    loc = gl.get_uniform_location(prog, 'view')
    mat4.identity(mat)
    local camX = 0 -- math.sin(time) * 10
    local camZ = 0 -- math.cos(time) * 10
    mat4.look_at(mat, {camX, 40, camZ - 80}, {0, 0, 0}, {0, 1, 0})
    gl.uniform_matrix4fv(loc, 1, false, mat)
    
    gl.bind_vertex_array(vao)
    
    for i,pos in ipairs(positions) do
        loc = gl.get_uniform_location(prog, 'model')
        mat4.translate(mat, pos[1], pos[2], pos[3])
        mat4.rotate(mat, mat, 0, 1.0, 0, math.rad(45) * time)
        mat4.rotate(mat, mat, 1, 0, 0, math.rad(270))
        gl.uniform_matrix4fv(loc, 1, false, mat)
        
        -- gl.drawElements(gl.TRIANGLES, 35288*mult, gl.UNSIGNED_INT, 0)
        -- gl.drawElements(gl.TRIANGLES, catModel.num_triangles, gl.UNSIGNED_INT, 0)
        gl.draw_arrays(gl.TRIANGLES, 0, catModel.num_vertices)
    end
    gl.bind_vertex_array()
    gl.use_program()
    gl.bind_texture(gl.TEXTURE_2D)
    gl.bind_buffer(gl.ELEMENT_ARRAY_BUFFER)
    
    -- render:setColor(75, 75, 125, 255)
    -- render:clear()
    -- render:present()
    window:gl_swap()
    sdl.delay(DELAY)
end
)


selene.set_quit(
function()
    gl.delete_textures(tex)
    gl.delete_buffers(buf)
    gl.delete_buffers(index)
    gl.delete_vertex_arrays(vao)
    gl.delete_program(prog)

    -- render:destroy()
    ctx:destroy()
    window:destroy()
    sdl.quit()
end
)
