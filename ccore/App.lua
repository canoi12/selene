local gl = selene.gl
local sdl = selene.sdl2
local traceback = debug.traceback

local Config = require('core.Config')
local Event = require('core.Event')
local Render = require('core.Render')
local Window = require('core.Window')
local Filesystem = require('core.Filesystem')

local App = {}
local App_mt = {
    __index = App
}

function App.create(config)
    local app = {}
    local args = selene.args

    app.config = config
    app.event = Event.create()
    return setmetatable(
        app,
        App_mt
    )
end

local FPS = 60.0
local DELAY = 1000.0 / FPS
local INIT_DELTA = 1 / FPS

function App:processCallback()
    local tp = self.event:getType()
    if tp == "quit" then
        selene.setRunning(false)
    elseif tp == "window event" then
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
    while e:poll() do
        self:processCallback()
    end
    
    while deltaTime > 0.0 do
        local dt = math.min(delta, default_delta)
        if self.update then self:update(dt) end
        deltaTime = deltaTime - dt
    end

    if self.draw then self:draw(self.render) end
    self.window:swap()
    sdl.delay(DELAY)
end

function App:destroy()
    self.render:destroy()
    self.window:destroy()
end

return App
