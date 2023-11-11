local App = require('core.App')
local Config = require('core.Config')
local Color = require('core.graphics.Color')
local Render = require('core.Render')

local config = Config.create("Knightvania", 640, 380)
local app = App.create(config)

function app:update(dt)
end

function app:draw(r)
    r:begin()
    r:clearColor(Color.black)
    r:clear()

    r:fillRectangle(0, 0, 128, 64)

    r:drawCircle(128, 128, 16)
    r:fillCircle(128, 192, 16)

    r:setDrawColor(Color.black)
    r:print("JÁ PODE FUMA?", 2, 2)
    r:finish()
end

return app
