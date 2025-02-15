sdl.init(sdl.INIT_EVERYTHING)
local profile = {
    major = 3,
    minor = 2,
    mask = sdl.GL_CONTEXT_PROFILE_CORE
}
if os.host() == 'android' then
    profile.major = 2
    profile.minor = 0
    profile.mask = sdl.GL_CONTEXT_PROFILE_ES
end
sdl.gl_set_attribute(sdl.GL_CONTEXT_MAJOR_VERSION, profile.major)
sdl.gl_set_attribute(sdl.GL_CONTEXT_MINOR_VERSION, profile.minor)
sdl.gl_set_attribute(sdl.GL_CONTEXT_PROFILE_MASK, profile.mask)
local window = sdl.create_window("SDL2 Test", sdl.WINDOWPOS_CENTERED, sdl.WINDOWPOS_CENTERED, 640, 380, sdl.WINDOW_SHOWN | sdl.WINDOW_OPENGL)
print(window)
-- Create and load modern GL context
local ctx = sdl.create_gl_context(window)
print(ctx)
gl.setup()

local event = sdl.create_event()

local current = sdl.get_ticks()
local last = sdl.get_ticks()

local buf = gl.gen_buffers(1)
local vert = gl.create_shader(gl.VERTEX_SHADER)
local frag = gl.create_shader(gl.FRAGMENT_SHADER)

local version = '#version 120\n'
if os.host() == 'android' then
    version = '#version 100\n'
end

gl.shader_source(vert, version .. [[
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
local precision = ''
if os.host() == 'android' then
    precision = 'precision mediump float;\n'
end
gl.shader_source(frag, version .. precision .. [[
varying vec3 v_color;
varying vec2 v_texcoord;

uniform sampler2D u_tex;

void main() {
    gl_FragColor = texture2D(u_tex, v_texcoord) * vec4(v_color, 1);
}
]])
gl.compile_shader(vert)
gl.compile_shader(frag)

local prog = gl.create_program()
gl.attach_shader(prog, vert)
gl.attach_shader(prog, frag)
gl.link_program(prog)

gl.use_program(prog)
local loc = gl.get_uniform_location(prog, 'world')

-- Ortho matrix
local m = linmath.mat4x4.alloc()
-- print(m, m.identity, linmath.mat4x4.identity)
m:identity()
m:ortho(0, 640, 380, 0, -1, 1)
gl.uniform_matrix4fv(loc, 1, false, m)

gl.use_program()

gl.delete_shader(vert)
gl.delete_shader(frag)

local xx = 0

-- Data for triangle
-- local data = selene.create_data(4 * 7 * 3)
-- data:write_floats(0, 320, 0, 1, 0, 1, 0.5, 0)
-- data:write_floats(28, 640, 380, 0, 1, 1, 1, 1)
-- data:write_floats(56, 0, 380, 1, 1, 0, 0, 1)

local values = {
    320, 0, 1, 0, 1, 0.5, 0,
    640, 380, 0, 1, 1, 1, 1,
    0, 380, 1, 1, 0, 0, 1
}

local data = ctypes.float(21)
for i,v in ipairs(values) do
    data[i-1] = v
end
print(data[0], data[1], data[2], data[3], data[4], data[5], data[6])
print(data[7], data[8], data[9], data[10], data[11], data[12], data[13])
print(data[14], data[15], data[16], data[17], data[18], data[19], data[20])

print(ctypes.float, ctypes.int8)
for i,v in pairs(ctypes) do
    print(i, v)
end
float = ctypes.float

local vao = gl.gen_vertex_arrays(1)

gl.bind_vertex_array(vao)
gl.bind_buffer(gl.ARRAY_BUFFER, buf)
gl.buffer_data(gl.ARRAY_BUFFER, 21*ctypes.float.size, data, gl.STATIC_DRAW)

gl.enable_vertex_attrib_array(0)
gl.enable_vertex_attrib_array(1)
gl.enable_vertex_attrib_array(2)
gl.vertex_attrib_pointer(0, 2, gl.FLOAT, false, 7*4, 0)
gl.vertex_attrib_pointer(1, 3, gl.FLOAT, false, 7*4, 2*4)
gl.vertex_attrib_pointer(2, 2, gl.FLOAT, false, 7*4, 5*4)

gl.bind_buffer(gl.ARRAY_BUFFER)
gl.bind_vertex_array()

local img_data = image.from_file(selene.__dir .. '/tuiteiro.jpg')
print(img_data.width, img_data.height, img_data.data, img_data.channels)

local tex = gl.gen_textures(1)
gl.bind_texture(gl.TEXTURE_2D, tex)
gl.tex_parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
gl.tex_image2d(gl.TEXTURE_2D, 0, gl.RGBA, img_data.width, img_data.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, img_data.data)
gl.bind_texture(gl.TEXTURE_2D)

selene.set_event(function(name)
    -- print(name)
    if name == 'quit' then selene.set_running(false)
    elseif name == 'window closed' then
        selene.set_running(false)
    end
end)

selene.set_step(
function()
    last = current
    current = sdl.get_ticks()
    local delta = (current - last) / 1000
    gl.clear_color(0.3, 0.3, 0.4, 1.0)
    gl.clear(gl.COLOR_BUFFER_BIT)

    gl.bind_texture(gl.TEXTURE_2D, tex)
    gl.use_program(prog)
    gl.bind_vertex_array(vao)
    gl.draw_arrays(gl.TRIANGLES, 0, 3)
    gl.bind_vertex_array()
    gl.use_program()
    gl.bind_texture(gl.TEXTURE_2D)

    window:gl_swap()
end
)

selene.set_quit(
function()
    print('quitting')
    gl.delete_textures(tex)
    gl.delete_buffers(buf)
    gl.delete_program(prog)
    gl.delete_vertex_arrays(vao)
    ctx:destroy()
    window:destroy()
    sdl.quit()
end
)
