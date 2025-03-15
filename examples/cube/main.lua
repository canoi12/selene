local conf = {
    window = {
        resizable = true
    }
}
selene('Cube Example', '1.0.0', 'org.selene.CubeExample', conf)
local renderer = selene.renderer.RenderBatch2D(selene.get_renderer())
local canvas = selene.renderer.create_canvas(160, 95)
print(renderer)
-- print(renderer, renderer.get_white_texture)
print(renderer.push_cube)
-- print(renderer:get_white_texture())

local x = 256
local y = 96
local z = -128

local last = sdl.get_ticks()
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
    local current = sdl.get_ticks()
    local delta = (current - last)
    --print(delta, current, last)
    last = current
    dt = delta / 1000
    time = time + dt
    local ay = math.rad(45)*time
    --print(ay, time)
    renderer:begin()
    renderer:enable_3d()
    renderer:clear(0.2, 0.3, 0.3)
    renderer:set_canvas(canvas)
    renderer:clear()
    renderer:push_cube({64, 24, -26}, {0, ay, math.rad(15)*time}, {32, 32, 32})
    renderer:push_rect(0, 0, 32, 32)
    renderer:set_color(1, 0, 0)
    renderer:push_rect(160-32, 95-32, 32, 32)
    renderer:set_color()
    renderer:set_texture(canvas)
    renderer:set_canvas()
    renderer:set_color()
    --renderer:set_canvas(canvas)
    renderer:push_cube({x, y, z}, {math.rad(x_angle), math.rad(y_angle), 0}, {160, 95, 160}, true)
    --renderer:set_canvas(nil)
    renderer:push_rect(0, 0, 160, 95)
    renderer:set_texture(renderer:get_white_texture())
    renderer:disable_3d()
    renderer:finish()
    renderer:present()
end)
selene.set_quit(function()
    renderer:destroy()
end)
