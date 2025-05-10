selene()
local win = selene.create_window('Renderer Example', 640, 380, {vulkan=true})
local ren = selene.create_renderer(win, 'vulkan')

local buffer = ren:create_buffer('vertex', 1024*1024)
print(buffer)

local batch = selene.renderer.VertexBatch(9*4, 1024)
batch:set_color(1, 1, 1, 1)
batch:set_z(0)

batch:push_vertex2d(0, 0.5, 0.2, 1, 0, 1, 1, 0, 0)
batch:push_vertex2d(0.5, -0.5, 0.2, 0, 1, 1, 1, 0, 0)
batch:push_vertex2d(-0.5, -0.5, 0.2, 1, 1, 0, 1, 0, 0)

ren:send_buffer_data(buffer, batch:get_offset()*batch:get_stride(), batch:get_data())

local f = io.open(selene.__dir .. '/vert.spv')
local size = f:seek('end')
f:seek('set')
local source = f:read('*a')
f:close()

local data, size = selene.filesystem.read(selene.__dir .. '/vert.spv', true)
print(size, data)
local vert = ren:create_shader('vertex', size, data)

data, size = selene.filesystem.read(selene.__dir .. '/frag.spv', true)

print(size, data)

local frag = ren:create_shader('pixel', size, data)

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
    scissor = {enabled = true}
}
print(pipeline)

selene.set_event(function(name)
    if name == 'quit' or name == 'window closed' then selene.set_running(false) end
end)

selene.set_step(function()
    ren:clear_color(0.3, 0.4, 0.4, 1.0)
    ren:set_pipeline(pipeline)
    ren:set_vertex_buffer(buffer)
    ren:draw('triangles', 0, 3)
    ren:present()
    selene.delay(16)
end)

selene.set_quit(function()
    batch:destroy()
    if pipeline then ren:destroy_pipeline(pipeline) end
    ren:destroy_shader(vert)
    ren:destroy_shader(frag)
    ren:destroy_buffer(buffer)
    ren:destroy()
    win:destroy()
end)
