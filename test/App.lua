local sdl = selene.sdl2
local traceback = debug.traceback
local AudioSystem = require('audio.AudioSystem')
local Event = require('core.Event')
local Render = require('core.graphics.Renderer')
local Window = require('core.Window')
local Filesystem = require('core.Filesystem')
local Settings = require('core.Settings')

--- @class App
--- @field audio AudioSystem
--- @field settings Settings
--- @field render Renderer
--- @field window Window
local App = {}
local app_mt = {}
app_mt.__index = App

function App.default()
    local app = {}
    local args = selene.args

    app.event = Event.create()
    app.execFs = Filesystem.create(sdl.getBasePath())
    if not args[2] then
        app.projectFs = app.execFs
    else
        app.projectFs = Filesystem.create(args[2])
    end
    app.userFs = Filesystem.create(sdl.getPrefPath(config.org, config.name))

    --- @type Settings
    local config = app.projectFs:load('.selene/settings.lua')()

    app.audio = AudioSystem.create(config.audio)

    app.window = Window.create(config)
    app.render = Render.create(app.window)
    app.settings = config
    -- app.audioSystem = audio.create(config)
    -- audio.setCurrent(app.audioSystem)
    
    return setmetatable(app, app_mt)
end

--- @param config Settings
function App.create(config)
    local app = {}
    local args = selene.args

    app.event = Event.create()
    app.execFs = Filesystem.create(sdl.getBasePath())
    if not args[2] then
        app.projectFs = app.execFs
    else
        app.projectFs = Filesystem.create(args[2])
    end
    app.userFs = Filesystem.create(sdl.getPrefPath(config.org, config.name))

    app.window = Window.create(config)
    app.render = Render.create(app.window)
    app.settings = config

    app.audio = AudioSystem.create(config)
    -- app.audioSystem = audio.create(config)
    -- audio.setCurrent(app.audioSystem)
    
    return setmetatable(app, app_mt)
end

local default_delta = 1 / 60.0
local FPS = 60.0
local DELAY = 1000.0 / FPS

function App:processCallback()
    local type_ = self.event:getType()
    if type_ == "quit" then
        selene.setRunning(false)
    elseif type_ == "window event" then
        local wid = self.event.handle:windowEvent()
        if wid == sdl.WINDOWEVENT_CLOSE then
            selene.setRunning(false)
        end
    end
end

local last = sdl.getTicks()
function App:step()
    local current = sdl.getTicks()
    local delta = (current - last)
    local deltaTime = delta / 1000
    last = current

    local e = self.event
    while e:poll() do self:processCallback() end

    while deltaTime > 0.0 do
        local dt = math.min(delta, default_delta)
        if self.update then self:update(dt) end
        deltaTime = deltaTime - dt
    end

    if self.draw then self:draw(self.render) end
    self.window:swap()
    sdl.delay(DELAY)
    return true
end

function App.createError(msg)
    local app = {}
    local args = selene.args
    local trace = traceback('', 1)
    print(msg, trace)
    app.settings = Settings.default()
    app.event = Event.create()

    app.execFs = Filesystem.create(sdl.getBasePath())
    if not args[2] then
        app.projectFs = app.execFs
    else
        app.projectFs = Filesystem.create(args[2])
    end
    app.userFs = Filesystem.create(sdl.getPrefPath('selene', 'app'))

    app.window = Window.create(app.config)
    app.render = Render.create(app.window)
    app.update = function() end
    app.draw = function() end
    return setmetatable(app, {
        __index = App
    })
end

function App:destroy()
    self.audio:destroy()
    self.render:destroy()
    self.window:destroy()
end

return App