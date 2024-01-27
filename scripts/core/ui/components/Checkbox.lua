local Frame = require('ui.Frame')
--- @class ui.component.Checkbox : ui.Component
--- @field value boolean
local Checkbox = {}
local check_mt = {}
check_mt.__index = Checkbox

--- @param id string
--- @param value boolean
--- @return ui.component.Checkbox
function Checkbox.create(id, value)
  --- @type ui.component.Checkbox
  local c = {
    id = id,
    value = true,
    state = "normal"
  }
  return setmetatable(c, check_mt)
end

--- @param r Renderer
--- @param x number
--- @param y number
--- @param style ui.Style
function Checkbox:draw(r, x0, y0, x1, y1, style)
  local st = style.checkbox[self.state] or style.checkbox.normal
  Frame.draw(r, x0, y0, x1, y1, st)
  if self.value then
    local icon = style.checkbox.icon
    r:setDrawColor(icon.shadow)
    r:print('X', x0 + 5, y0 + 4)
    r:setDrawColor(icon.fg)
    r:print('X', x0 + 5, y0 + 3)
  end
end

function Checkbox:onEvent(e)
end

return Checkbox
