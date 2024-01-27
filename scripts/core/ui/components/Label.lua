--- @class ui.component.Label : ui.Component
--- @field text string
local Label = {}
local label_mt = {}
label_mt.__index = Label

function Label.create(id, label)
  local l = {
    id = id,
    text = label,
    state = "normal"
  }
  return setmetatable(l, label_mt)
end

--- @param r Renderer
--- @param x0 number
--- @param y0 number
--- @param x1 number
--- @param y1 number
--- @param style ui.Style
function Label:draw(r, x0, y0, x1, y1, style)
  r:setDrawColor(style.text.shadow)
  r:print(self.text, x0 + 4, y0 + 4)
  r:setDrawColor(style.text.fg)
  r:print(self.text, x0 + 3, y0 + 3)
end

return Label
