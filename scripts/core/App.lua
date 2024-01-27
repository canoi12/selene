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
--- @field ui ui.Context | nil
--- @field assetManager AssetManager | nil
--- @field onEvent function | nil
--- @field projectFs Filesystem
--- @field onTick function | nil
--- @field onRender function | nil
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
    config = Settings.default()
    --[[
    local state, engine = pcall(function() return require('engine') end)
    if state then
        config = engine.setup(app)
    else
        error(engine)
    end
    --]]
    app.onTick = function(_, dt)
    end
    app.onRender = function(_, r)
        r:clearColor(Color.black)
        r:clear()
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

--- @param paletteName ui.DefaultPalette | ui.Palette
function App:initUI(paletteName)
    local ui = require('ui')
    local palettes = require('ui.palettes')
    local Style = require('ui.Style')
    local palette = {}
    if type(paletteName) == "string" then
        palette = palettes[paletteName]
    elseif type(paletteName) == "table" then
        palette = paletteName
    end
    self.ui = ui.create(Style.create(palette))
end

local default_delta = 1 / 60.0
local FPS = 60.0
local DELAY = 1000.0 / FPS

--- @param e Event
function App:processCallback(e)
    local type_ = self.event:getType()
    if type_ == "quit" then
        selene.setRunning(false)
    elseif type_ == "window event" then
        local wev, wid, d1, d2 = self.event.handle:windowEvent()
        if wev == sdl.WINDOWEVENT_CLOSE then
            selene.setRunning(false)
        elseif wev == sdl.WINDOWEVENT_RESIZED then
            self.render:onResize(d1, d2)
        end
    end
    if self.ui then self.ui:onEvent(e) end
    if self.onEvent then self:onEvent(e) end
end

local last = sdl.getTicks()
function App:step()
    self.audio:update()
    local current = sdl.getTicks()
    local delta = (current - last)
    local deltaTime = delta / 1000
    last = current

    local e = self.event
    while e:poll() do self:processCallback(e) end

    while deltaTime > 0.0 do
        local dt = math.min(delta, default_delta)
        if self.onTick then self:onTick(dt) end
        deltaTime = deltaTime - dt
    end

    self.render:begin()
    if self.onRender then self:onRender(self.render) end
    if self.ui then self.ui:render(self.render) end
    self.render:finish()
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
    app.onTick = function() end
    app.onRender = function() end
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
