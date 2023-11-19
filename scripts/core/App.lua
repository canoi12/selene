--- @type selene.sdl2
local sdl = selene.sdl2
local traceback = debug.traceback
local AudioSystem = require('audio.AudioSystem')
local Color = require('graphics.Color')
local Event = require('core.Event')
local Render = require('core.graphics.Renderer')
local Window = require('Window')
local Filesystem = require('core.Filesystem')
local Settings = require('core.Settings')

--- @class App
--- @field audio AudioSystem
--- @field settings Settings
--- @field render Renderer
--- @field window Window
--- @field event Event
--- @field assetManager AssetManager | nil
local App = {}
local app_mt = {}
app_mt.__index = App

--- @return App
function App.defaultEngine()
    local app = {}
    local args = selene.args

    app.event = Event.create()
    app.execFs = Filesystem.create(sdl.getBasePath())
    if not args[2] then
        app.projectFs = app.execFs
    else
        app.projectFs = Filesystem.create(args[2])
    end
    local config = {}
    local state, engine = pcall(function() return require('engine') end)
    if state then
        config = engine.setup(app)
    else
        error(engine)
    end
    local org = config.org or "selene"
    app.userFs = Filesystem.create(sdl.getPrefPath(org, config.name))

    app.audio = AudioSystem.create(config)
    app.window = Window.create(config)
    app.render = Render.create(app.window)
    app.settings = config

    return setmetatable(app, app_mt)
end

--- @param config Settings
--- @return App
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
    local org = config.org or "selene"
    app.userFs = Filesystem.create(sdl.getPrefPath(org, config.name))

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
    self.audio:update()
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
