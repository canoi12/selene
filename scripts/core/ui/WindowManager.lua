--- @class ui.WindowManager
--- @field 
--- @field windows ui.Window[]
--- @field hoveredWindow ui.Window | nil
--- @field activeWindow ui.Window | nil
--- @field editWindow {current:ui.Window|nil,state:string}
local WindowManager = {}
local wm_mt = {}
wm_mt.__index = WindowManager

--- @param ui ui.Context
--- @return ui.WindowManager
function WindowManager.create(ui)
  local wm = {}
  wm.windows = {}
  wm.hoveredWindow = nil
  wm.activeWindow = nil
  wm.editWindow = {
    current = nil,
    state = 'none'
  }
  return setmetatable(wm, wm_mt)
end

--- Event ou mouse motion
---@param x number
---@param y number
---@param dx number
---@param dy number
function WindowManager:onMouseMotion(x, y, dx, dy)
  if self.editWindow.current and self.editWindow.state ~= 'none' then
    --- @type ui.Window
    local win = self.editWindow.current
    if self.editWindow.state == "move" then
      win.rect.x = win.rect.x + dx
      win.rect.y = win.rect.y + dy
    elseif self.editWindow.state == "resize" then
      win.rect.w = math.max(win.rect.w + dx, 64)
      win.rect.h = math.max(win.rect.h + dy, 64)
    end
  end
end

function WindowManager:onMouseButton(btn, pressed, x, y)
end

return WindowManager
