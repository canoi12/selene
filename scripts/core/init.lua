local sdl = selene.sdl2
local traceback = debug.traceback

local AudioSystem = require('core.audio.AudioSystem')
local Event = require('core.Event')
local Filesystem = require('core.Filesystem')
local Settings = require('core.Settings')
local Renderer = require('core.graphics.Renderer')
local Window = require('core.Window')

--- @class core.Context
--- @field audio AudioSystem
--- @field render core.Renderer
--- @field window Window
--- @field onStart function | nil
--- @field onEvent function | nil
--- @field onUpdate function | nil
--- @field onRender function | nil
--- @field onQuit function | nil
local core = {}

if not sdl.init(
    sdl.INIT_SENSOR, sdl.INIT_AUDIO,
    sdl.INIT_VIDEO, sdl.INIT_TIMER,
    sdl.INIT_JOYSTICK, sdl.INIT_GAMECONTROLLER,
    sdl.INIT_EVENTS
) then
    error("Failed to init SDL2: " .. sdl.getError())
end

function core.init(settings)
    local args = selene.args
    core.event = Event.create()
    core.execFs = Filesystem.create(sdl.getBasePath())
    if not args[2] then
        core.projectFs = core.execFs
    else
        core.projectFs = Filesystem.create(args[2])
    end
    settings = settings or Settings.default()

    local org = settings.org or 'selene'
    core.userFs = Filesystem.create(sdl.getPrefPath(org, settings.name))

    local win = Window.create(settings)
    if win == nil then
        error('Failed to create window: ' .. sdl.getError())
    end
    core.window = win
    core.render = Renderer.create(win)
    core.settings = settings

    core.audio = AudioSystem.create(settings)
    return core
end

local default_delta = 1 / 60.0
local FPS = 60.0
local DELAY = 1000.0 / FPS

--- @param e Event
function core.processCallback(e)
    local type_ = core.event:getType()
    if type_ == 'quit' then
        selene.setRunning(false)
    elseif type_ == 'window event' then
        local wev, wid, d1, d2 = e.handle:windowEvent()
        if wev == sdl.WINDOWEVENT_CLOSE then
            selene.setRunning(false)
        elseif wev == sdl.WINDOWEVENT_RESIZED then
            core.render:onResize(d1, d2)
        end
    end
    if core.onEvent then core.onEvent(e) end
end

function core.default()
    if core.onStart then core.onStart() end
    local last = sdl.getTicks()
    return {
        step = function()
            core.audio:update()
            local current = sdl.getTicks()
            local delta = (current - last)
            local deltaTime = delta / 1000
            last = current

            local e = core.event
            while e:poll() do core.processCallback(e) end

            while deltaTime > 0.0 do
                local dt = math.min(delta, default_delta)
                if core.onUpdate then core.onUpdate(dt) end
                deltaTime = deltaTime - dt
            end

            core.render:begin()
            if core.onRender then core.onRender(core.render) end
            core.render:finish()
            core.window:swap()
            sdl.delay(DELAY)
            return true
        end,
        quit = function()
            if core.onQuit then core.onQuit() end
            core.audio:destroy()
            core.render:destroy()
            core.window:destroy()
        end
    }
end

return core


--[[
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
    if app.onInit then app:onInit() end
else
    app = App.createError(ret)
end

selene.app = app

return  {
    step = function() app:step() end,
    quit = function() app:destroy() end
}
]]

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
