selene('My Game', '1.0.0', 'org.selene.MyGame')
print(selene.renderer)
print(selene.get_renderer)
local render = selene.renderer.RenderBatch2D(selene.get_renderer())
if not selene then
    error(err)
end
print(selene, selene.__version)
print(_BOOT_SCRIPT)
print(_VERSION)
local i = 1
local img = image.from_file('selene_icon.png')
local tex = selene.renderer.create_texture(img.width, img.height, 'rgba', img.data)
local t = 0
selene.set_event(function(name, ...)
    if name == 'quit' or name == 'window close' then
        selene.set_running(false)
    end
    if name == 'gamepad added' then
        local which = ...
        sdl.gamepad_open(which)
    elseif name == 'gamepad button' then
        local which,button = ...
        print('button', which, button)
    elseif name == 'gamepad axis' then
        local which,axis,value = ...
        print('axis', which, axis, value)
    end
end)
selene.set_step(function()
    t = t + 0.5
    if t > 360 then
        t = 0
    end
    render:begin()
    render:clear(0.2, 0.3, 0.3)
    render:set_blend_mode('alpha')
    render:set_color(1, 0, 1)
    render:push_triangle(320, 0, 0, 320, 640, 320)
    render:set_color(0, 1, 1)
    render:push_rect(256, 320, 8, 8)
    render:set_texture(tex)
    render:set_color(1, 1, 1)
    render:push_sprite(256, 320, t, 0.5, 0.5, {256, 256}, {true, true})
    render:finish()
    render:present()
end)
