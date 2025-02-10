selene('My Game', '1.0.0', 'org.selene.MyGame')
print(selene.renderer, selene.renderer.Batch2D)
if not selene then
    error(err)
end
print(selene, selene.__version)
print(_BOOT_SCRIPT)
print(_VERSION)
local i = 1
selene.set_event(function(name, ...)
    if name == 'quit' then selene.set_running(false)
    elseif name == 'window close' then
        selene.set_running(false)
    end
end)
selene.set_step(function()
end)