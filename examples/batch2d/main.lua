local RenderBatch2D = require('plugins.RenderBatch2D')
print(RenderBatch2D, RenderBatch2D.create)

selene()
local win = selene.create_window('batch 2D', 640, 380, {opengl = true})
local ren = RenderBatch2D.create(win)


selene.set_event(function(name)
    if name == 'quit' or name == 'window closed' then selene.set_running(false) end
end)

selene.set_step(function()
    ren:begin_frame()
    ren:clear(0.3, 0.4, 0.4, 1.0)
    -- ren:set_color(1, 1, 1, 1)
    ren:draw_circle(128, 96, 32)
    ren:end_frame()
end)

selene.set_quit(function()
    ren:destroy()
    win:destroy()
end)