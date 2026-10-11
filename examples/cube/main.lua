local conf = {
    window = {
        resizable = false
    }
}
local RenderBatch2D = require('plugins.RenderBatch2D')
local Canvas = require('plugins.RenderBatch2D.Canvas')

local backend = 'opengl'

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
local title = 'Renderer ' .. backend .. ' example (New API with RenderBatch2D)'
local win = selene.create_window(title, 640, 380, {opengl=opengl, vulkan=vulkan})
local render = RenderBatch2D.create(win, backend)
print(render)
local canvas = Canvas.create(render, 160, 95)
print(render)

local x = 256
local y = 96
local z = -128

local last = selene.get_ticks()
local time = 0
local y_angle = 0
local x_angle = 0

local dt = 0
local keys = {}

local gpads = {}
selene.set_event(function(name, ...)
    if name == 'quit' or name == 'window closed' then
        selene.set_running(false)
    end

    if name == 'key' then
        local key, pressed = ...
        keys[string.lower(key)] = pressed
    end
end)

selene.set_step(function()
    if keys.left then
        x = x - (80*dt)
    elseif keys.right then
        x = x + (80*dt)
    end

    if keys.up then
        z = z - (80*dt)
    elseif keys.down then
        z = z + (80*dt)
    end

    if keys.a then
        y_angle = y_angle + (60*dt)
    elseif keys.d then
        y_angle = y_angle - (60*dt)
    end
    if keys.w then
        x_angle = x_angle + (60*dt)
    elseif keys.s then
        x_angle = x_angle - (60*dt)
    end
    local current = selene.get_ticks()
    local delta = (current - last)
    --print(delta, current, last)
    last = current
    dt = delta / 1000
    time = time + dt
    local ay = math.rad(45)*time
    --print(ay, time)
    render:begin_frame()
    -- Pass 1: 3D scene into the canvas (target must be set at begin_pass)
    render:begin_pass({
        target = canvas.target,
        clear_color = {0.2, 0.3, 0.3, 1.0},
        clear_depth = 1.0,
    })
    render:enable_3d()
    render:draw_cube({64, 24, -26}, {0, ay, math.rad(15)*time}, {32, 32, 32}, true)
    render:draw_rectangle(0, 0, 8, 8)
    render:set_color(1, 0, 0)
    render:draw_rectangle(160-32, 95-32, 32, 32)
    render:set_color(1, 1, 1)
    render:enable_3d(false)
    render:end_pass()
    -- Pass 2: draw the canvas onto the default framebuffer
    render:begin_pass({
        clear_color = {0.2, 0.3, 0.3, 1.0},
    })
    render:draw(canvas, 0, 0, 0, 4, 4)
    render:end_pass()
    render:end_frame()
    render:present()
end)
selene.set_quit(function()
    render:destroy()
end)
