selene('My Game', '1.0.0', 'org.selene.MyGame')
print(selene.renderer, selene.renderer.Batch2D)
print(selene.get_renderer)
local renderer = selene.get_renderer()
if not selene then
    error(err)
end
print(selene, selene.__version)
print(_BOOT_SCRIPT)
print(_VERSION)
local i = 1
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
    renderer:clear(0.2, 0.3, 0.3)
    renderer:push_triangle(0, 0.5, 0.5, -0.5, -0.5, -0.5)
end)
