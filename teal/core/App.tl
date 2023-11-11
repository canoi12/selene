local sdl = selene.sdl2
local traceback = debug.traceback
local Config = require('core.Config')
local Event = require('core.Event')
local Render = require('core.Render')
local Window = require('core.Window')
local Filesystem = require('core.Filesystem')

local record App
    config: Config
    event: Event
    execFs: Filesystem
    projectFs: Filesystem
    userFs: Filesystem

    window: Window
    render: Render

    create: function(Config) : App
    createError: function(string) : App

    destroy: function(App)

    processCallback: function(App)

    update: function(App, number)
    draw: function(App, Render)

    step: function(App) : boolean
end

function App.create(config: Config) : App
    local app : App = {}
    local args = selene.args

    app.config = config

    app.event = Event.create()
    app.execFs = Filesystem.create(sdl.getBasePath())
    if not args[2] then
        app.projectFs = app.execFs
    else
        app.projectFs = Filesystem.create(args[2])
    end
    app.userFs = Filesystem.create(sdl.getPrefPath(config.info.org, config.info.name))

    app.window = Window.create(config)
    app.render = Render.create(app.window)

    return setmetatable(app, {
        __index = App
    })
end

local default_delta <const> : number = 1 / 60.0
local FPS <const> = 60.0
local DELAY <const> = 1000.0 / FPS

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
function App:step() : boolean
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

function App.createError(msg: string) : App
    local app : App = {}
    local args = selene.args
    local trace = traceback('', 1)
    print(msg, trace)
    app.config = Config.default()
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
    app.update = function(s: App) end
    app.draw = function(s: App) end
    return setmetatable(app, {
        __index = App
    })
end

function App:destroy()
    self.render:destroy()
    self.window:destroy()
end

return App