local sdl = selene.sdl2
local traceback = debug.traceback

local function add_path(path)
    package.path = path .. '?.lua;' ..
        path .. '?/init.lua;' ..
        package.path
end

add_path(sdl.getBasePath() .. 'core/')

--- @type App
local App = require('App')

--- @class Core
local core = {}

if not sdl.init(
    sdl.INIT_SENSOR, sdl.INIT_AUDIO,
    sdl.INIT_VIDEO, sdl.INIT_TIMER,
    sdl.INIT_JOYSTICK, sdl.INIT_GAMECONTROLLER,
    sdl.INIT_EVENTS
) then
    error("Failed to init SDL2: " .. sdl.getError())
end

local args = selene.args
local projectPath = "./"
if args[2] then
    projectPath = args[2]
    package.path = projectPath .. '?.lua;' ..
        projectPath .. '?/init.lua;' ..
        package.path
end

local app = {}

local state, ret = pcall(function() return require('main') end)
if state then
    app = ret
else
    app = App.createError(ret)
end

selene.app = app

return  {
    step = function() app:step() end,
    quit = function() app:destroy() end
}

--[[
function core.init(args: {string}) : App
    if not sdl.init(
        sdl.INIT_SENSOR, sdl.INIT_AUDIO,
        sdl.INIT_VIDEO, sdl.INIT_TIMER,
        sdl.INIT_JOYSTICK, sdl.INIT_GAMECONTROLLER,
        sdl.INIT_EVENTS
    ) then
        error("Failed to init SDL2: " .. sdl.getError())
    end
    local projectPath : string = args[2] or "./"
    package.path = projectPath .. '?.lua;' ..
        projectPath .. '?/init.lua;' ..
        package.path

    local state, ret = pcall(function() : App return require('main') end)
    if state then
        return ret
    end
    return App.createError(ret as string)
end

function core.step() : boolean
    xpcall(core.app.step, core.app.error)
end

function core:quit()
    core.app:destroy()
end


return core]]
