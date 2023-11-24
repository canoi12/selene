local Frame = require('engine.ui.Frame')
--- @class ui.Window
--- @field title string
--- @field rect Rect
--- @field style ui.WindowStyle
--- @field isOpen boolean
local Window = {}
local window_mt = {}
window_mt.__index = Window

--- @param title string
--- @param rect Rect
--- @param style ui.WindowStyle
--- @return ui.Window
function Window.create(title, rect, style)
    local w = {}
    w.title = title
    w.rect = rect
    w.style = style
    w.isOpen = true
    return setmetatable(w, window_mt)
end

--- @param r Renderer
function Window:draw(r)
    local x0, y0, x1, y1 = 0, 0, 0, 0
    x0 = self.rect.x
    y0 = self.rect.y
    x1 = self.rect.x + self.rect.w
    y1 = self.rect.y + self.rect.h

    Frame.draw(r, x0, y0, x1, y0+16, self.style.titleBar)
    Frame.draw(r, x0, y0+17, x1, y1, self.style)
end

return Window