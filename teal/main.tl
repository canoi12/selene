local App = require 'core.App'
local Config = require 'core.Config'
local Color = require('core.graphics.Color')
local Render = require('core.Render')

local config : Config = Config.create("Knightvania", 640, 380)
local app = App.create(config)

function app:update(_dt: number)
end

function app:draw(r: Render)
    r:clearColor(Color.black)
    r:clear()
end

return app