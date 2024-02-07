local App = require 'App'
local Settings = require 'Settings'
local Color = require 'graphics.Color'
local Canvas = require 'graphics.Canvas'
local Keyboard = require 'input.Keyboard'
local Gamepad = require 'input.Gamepad'
local app = App.create(Settings.default())

local canvas = Canvas.create(160, 95)

local rr = {x = 0, y = 0, w = 640, h = 380}
local x = 0
local y = 0

local gpad = Gamepad.open(0)
print(Gamepad.getButtonFromString("a"))

--- @param dt number
function app:onTick(dt)
    if gpad:getButton(0) == 1 then
        print("AAAAAAAAAAH")
    end
    if Keyboard.isDown("left") then
        x = x - (80*dt)
    elseif Keyboard.isDown("right") then
        x = x + (80*dt)
    end
end

--- @param r core.Renderer
function app:onRender(r)
    r:setClearColor(Color.red)
    r:clear()
    r:setCanvas(canvas)
    r:setClearColor()
    r:clear()
    r:print("Teste")
    r:setCanvas()
    r:blit(canvas)
    r:fillRectangle(x, 64, 128, 32)
end

return app