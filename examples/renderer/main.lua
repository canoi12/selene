local backend = 'vulkan'

local opengl = false
local vulkan = false

if backend == 'opengl' then
    opengl = true
    vulkan = false
elseif backend == 'vulkan' then
    opengl = false
    vulkan = true
end

selene{meta={org='Selene', name='App'}}

print(filesystem.resolve('exec://'))
print(filesystem.resolve('root://'))
print(filesystem.resolve('user://'))
print(filesystem.exists('user://'))

local title = 'Renderer ' .. backend .. ' example'
local win = selene.create_window(title, 640, 380, {opengl=opengl, vulkan=vulkan})
local ren = selene.create_renderer(win, backend)

print(win, ren)

local buffer = ren:create_buffer('vertex', 1024*1024)
print(buffer)

-- local pixels, size = selene.filesystem.read('selene_icon.png', true)
-- local image = image.from_memory(pixels, size)
-- print(pixels, size)

-- local tex = ren:create_texture2d(image.width, image.height, 'rgba', image.data)
local tex, str = ren:load_texture2d('root://selene_icon.png')
if not tex then
    --error(str)
    print(tex, str)
end
print(tex)

local batch = renderer.VertexBatch(9*4, 1024)
batch:set_color(1, 1, 1, 1)
batch:set_z(0)

-- batch:push_vertex2d(0, 0.5, 0, 1, 0, 1, 1, 0.5, 0)
-- batch:push_vertex2d(0.5, -0.5, 0, 0, 1, 1, 1, 1.0, 1.0)
-- batch:push_vertex2d(-0.5, -0.5, 0, 1, 1, 0, 1, 0, 1.0)

batch:push_vertex2d(-0.5, 0.5, 0, 1, 0, 1, 1, 0, 0)
batch:push_vertex2d(0.5, 0.5, 0, 0, 1, 1, 1, 1, 0)
batch:push_vertex2d(0.5, -0.5, 0, 1, 1, 0, 1, 1, 1)

batch:push_vertex2d(-0.5, 0.5, 0, 1, 0, 1, 1, 0, 0)
batch:push_vertex2d(0.5, -0.5, 0, 1, 1, 0, 1, 1, 1)
batch:push_vertex2d(-0.5, -0.5, 0, 0, 1, 1, 1, 0, 1)

ren:send_buffer_data(buffer, batch:get_offset()*batch:get_stride(), batch:get_data())

-- local f = io.open(selene.__dir .. '/shaders/vert.hlsl')
-- local size = f:seek('end')
-- f:seek('set')
-- local source = f:read('*a')
-- f:close()
-- print(source)

-- local data, size = selene.filesystem.read(selene.__dir .. '/shaders/shader.vert', true)
-- print(size, data)
-- local vert = ren:create_shader('vertex', size, data)

local info = ren:get_info()

local vert_file = 'root://shaders/vert.hlsl'
if backend == 'opengl' then
    vert_file = 'root://shaders/vert.glsl'
    if info.es then
        vert_file = 'root://shaders/vert.glsl100'
    end
elseif backend == 'vulkan' then
    vert_file = 'root://shaders/vert.spv'
end
local vert = ren:load_shader('vertex', vert_file)

-- data, size = selene.filesystem.read(selene.__dir .. '/shaders/shader.frag', true)
-- print(size, data)
-- local frag = ren:create_shader('pixel', size, data)
local frag_file = 'root://shaders/frag.hlsl'
if backend == 'opengl' then
    frag_file = 'root://shaders/frag.glsl'
    if info.es then
        frag_file = 'root://shaders/frag.glsl100'
    end
elseif backend == 'vulkan' then
    frag_file = 'root://shaders/frag.spv'
end
local frag = ren:load_shader('pixel', frag_file)


print(vert, frag)

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
    bindings = {
        {slot = 0, type = 'texture', stage = 'fragment'}
    }
}
print(pipeline)

selene.set_event(function(name)
    if name == 'quit' or name == 'window closed' then selene.set_running(false) end
end)

selene.set_step(function()
    ren:begin_frame()
    ren:begin_pass({
        clear_color = {0.3, 0.4, 0.4, 1.0},
        clear_depth = 1,
        viewport = {0, 0, 640, 380},
        scissor = {0, 0, 640, 380},
    })
    ren:set_pipeline(pipeline)
    ren:set_vertex_buffer(buffer)
    ren:set_texture(tex)
    ren:draw('triangles', 0, 6)
    ren:end_pass()
    ren:end_frame()
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
