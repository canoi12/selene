local Rect = require('Rect')
--- @class editor.components.Button
--- @field text string
--- @field action function
--- @field rect Rect
local Button = {}

local button_mt = {}
button_mt.__index = Button

function Button.create(text, action)
    local b = {}
    self.text = text
    self.action = action
    self.rect = Rect.create(0, 0)
end

function Button:draw()
end


return Button