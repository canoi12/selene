local sdl = selene.sdl2

local function add_path(path)
    package.path = path .. '?.lua;' .. path .. '?/init.lua;' .. package.path
end

return function(args)
    local core = {}
    add_path(sdl.GetBasepath())
    add_path(args[2])
    return require 'core'
end
