--- @type App
local App = require('App')
local Settings = require('Settings')
local Color = require('graphics.Color')

local Mouse = require('input.Mouse')
local Keyboard = require('input.Keyboard')

local settings = Settings.create("Knightvania", 640, 380)
local app = App.create(settings)

local x = 0
function app:update(dt)
    if Mouse.isDown(1) then
        print('carrrrrrrrrrrrralho')
    end
    if Keyboard.isDown('space') then
        x = x + (80 * dt)
    elseif Keyboard.isDown('backspace') then
        x = x - (80 * dt)
    end
end

function app:draw(r)
    r:begin()
    r:clearColor(Color.black)
    r:clear()

    r:fillRectangle(0, 0, 128, 64)

    r:drawCircle(x, 128, 16)
    r:fillCircle(x, 192, 16)

    r:setDrawColor(Color.black)
    r:print("JÁ PODE FUMA?", 2, 2)
    r:finish()
end

return app
