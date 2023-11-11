local sdl = selene.sdl2
local function add_path(path)
    package.path = path .. '?.lua;' .. path .. '?/init.lua;' .. package.path
end
return function(args)
    local core = {}
    local state = xpcall(function() require('tl').loader() end)
    if not state then
        print('Failed to load Teal')
    end
    add_path(sdl.GetBasepath())
    add_path(args[2])
    return require 'core'
end
