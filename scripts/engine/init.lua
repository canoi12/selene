local sdl = selene.sdl2
local traceback = debug.traceback

local AudioSystem = require('engine.audio.AudioSystem')
local Event = require('engine.Event')
local Filesystem = require('engine.Filesystem')
local Settings = require('engine.Settings')
local Renderer = require('engine.graphics.Renderer')
local Window = require('engine.Window')

--- @class engine.Context
--- @field audio AudioSystem
--- @field render core.Renderer
--- @field window Window
--- @field onStart function | nil
--- @field onEvent function | nil
--- @field onUpdate function | nil
--- @field onRender function | nil
--- @field onQuit function | nil
local engine = {}

if not sdl.init(
    sdl.INIT_SENSOR, sdl.INIT_AUDIO,
    sdl.INIT_VIDEO, sdl.INIT_TIMER,
    sdl.INIT_JOYSTICK, sdl.INIT_GAMECONTROLLER,
    sdl.INIT_EVENTS
) then
    error("Failed to init SDL2: " .. sdl.getError())
end

function engine.init(settings)
    local args = selene.args
    engine.event = Event.create()
    engine.execFs = Filesystem.create(sdl.getBasePath())
    if not args[2] then
        engine.projectFs = engine.execFs
    else
        engine.projectFs = Filesystem.create(args[2])
    end
    settings = settings or Settings.default()

    local org = settings.org or 'selene'
    engine.userFs = Filesystem.create(sdl.getPrefPath(org, settings.name))

    local win = Window.create(settings)
    if win == nil then
        error('Failed to create window: ' .. sdl.getError())
    end
    engine.window = win
    engine.render = Renderer.create(win)
    engine.settings = settings

    engine.audio = AudioSystem.create(settings)
    return engine
end

local default_delta = 1 / 60.0
local FPS = 60.0
local DELAY = 1000.0 / FPS

--- @param e Event
function engine.processCallback(e)
    local type_ = engine.event:getType()
    if type_ == 'quit' then
        selene.setRunning(false)
    elseif type_ == 'window event' then
        local wev, wid, d1, d2 = e.handle:windowEvent()
        if wev == sdl.WINDOWEVENT_CLOSE then
            selene.setRunning(false)
        elseif wev == sdl.WINDOWEVENT_RESIZED then
            engine.render:onResize(d1, d2)
        end
    end
    if engine.onEvent then engine.onEvent(e) end
end

function engine.default()
    if engine.onStart then engine.onStart() end
    local last = sdl.getTicks()
    return {
        step = function()
            engine.audio:update()
            local current = sdl.getTicks()
            local delta = (current - last)
            local deltaTime = delta / 1000
            last = current

            local e = engine.event
            while e:poll() do engine.processCallback(e) end

            while deltaTime > 0.0 do
                local dt = math.min(delta, default_delta)
                if engine.onUpdate then engine.onUpdate(dt) end
                deltaTime = deltaTime - dt
            end

            engine.render:begin()
            if engine.onRender then engine.onRender(engine.render) end
            engine.render:finish()
            engine.window:swap()
            sdl.delay(DELAY)
            return true
        end,
        quit = function()
            if engine.onQuit then engine.onQuit() end
            engine.audio:destroy()
            engine.render:destroy()
            engine.window:destroy()
        end
    }
end

return engine