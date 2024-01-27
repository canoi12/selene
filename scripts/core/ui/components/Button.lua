local Frame = require('ui.Frame')
--- @alias ui.ButtonState
---| "normal"
---| "hovered"
---| "pressed"
---| "released"

--- @class Button

--- @class ui.component.Button : ui.Component
--- @field label string
--- @field state ui.ButtonState
--- @field pressed boolean
local Button = {}
local button_mt = {}
button_mt.__index = Button

--- @param id string
--- @param label string
--- @return ui.component.Button
function Button.create(id, label)
  --- @type ui.component.Button
  local b = {
    id = id,
    label = label,
    state = "normal",
    pressed = false
  }
  return setmetatable(b, button_mt)
end

--- @param r Renderer
--- @param x0 number
--- @param y0 number
--- @param x1 number
--- @param y1 number
--- @param style ui.Style
function Button:draw(r, x0, y0, x1, y1, style)
    local btn = style.button
    local s = btn[self.state]
    Frame.draw(r, x0, y0, x1, y1, s)
    r:setDrawColor(style.text.shadow)
    r:print(self.label, x0 + 5, y0 + 5)
    r:setDrawColor(style.text.fg)
    r:print(self.label, x0 + 4, y0 + 4)
end

return Button
