selene()
local RenderBatch2D = require('plugins.RenderBatch2D')
local Canvas = require('plugins.RenderBatch2D.Canvas')
local Image = require('plugins.RenderBatch2D.Image')
print("Testando")
--selene('Testing', '1.0.0', 'org.selene.Testing', {renderer = {backend = 'opengl'}})
--local ctx = selene.get_context()
local win = selene.create_window('Testing', 640, 380, {resizable = true, opengl = true})
local rb2d = RenderBatch2D.create(win, 'opengl')
--local r = ctx.renderer
print('Renderer backend:', rb2d.backend)--ctx.renderer)
local x = 0
local y = 32
local keys = {}

local image = Image.load(rb2d, 'selene_icon.png')
local canvas = Canvas.create(rb2d, 320, 190, true)

selene.set_event(function(name, ...)
    if name == 'quit' or name == 'window closed' then selene.set_running(false) end
    if name == 'key' then
        local key, pressed = ...
        keys[string.lower(key)] = pressed
    end
    if name == 'window resized' then
        local w, h = ...
        rb2d:on_resize(w, h)
    end
end)
rb2d.delta = 0
rb2d.time = 0

local last = selene.get_ticks()
selene.set_step(function()
    if keys['left'] then
        x = x - 8
    elseif keys['right'] then
        x = x + 8
    end
    local current = selene.get_ticks()
    rb2d.delta = (current - last) / 1000
    last = current
    rb2d.time = rb2d.time + rb2d.delta
    
    rb2d:begin_frame()
    --[[rb2d:set_target(canvas.target)
    rb2d:clear(0.3, 0.4, 0.4, 1.0)
    rb2d:draw_triangle(320, 0, 0, 380, 640, 380)
    -- rb2d:draw_rectangle(196, 32, 192, 256)
    rb2d:draw_circle(x, y, 16)
    rb2d:enable_3d(true)
    rb2d:draw_cube(x+128, y+32)
    rb2d:draw_sphere(x+128, y+96, 32)
    rb2d:enable_3d(false)
    rb2d:set_target()]]
    rb2d:set_canvas(canvas)
    rb2d:clear(0.3, 0.4, 0.4, 1.0)
    rb2d:draw_triangle(320, 0, 0, 380, 640, 380)
    rb2d:enable_3d(true)
    rb2d:draw_cube(x+128, y+32)
    rb2d:draw_sphere(x+128, y+96, 32)
    rb2d:enable_3d(false)
    rb2d:set_canvas()
    -- rb2d:draw_sprite(canvas.texture, 0, 0, 0, 1, 1, nil, {false, true})
    rb2d:clear(0, 0, 0, 1.0)

    rb2d:draw(canvas, 0, 0)
    rb2d:set_clip_rect(0, 0, 320, 190)
    rb2d:draw(image, 32, 32)
    rb2d:set_clip_rect()
    rb2d:draw_text(rb2d.font, 'olaaar', 0, 0)

    rb2d:end_frame()
    selene.delay(16)
end)
selene.set_quit(function()
    image:destroy(rb2d)
    canvas:destroy(rb2d)
    rb2d:destroy()
    win:destroy()
end)