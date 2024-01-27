local Frame = require('ui.Frame')
local Label = require('ui.components.Label')
local Vec2 = require('math.Vec2')
--- @class ui.Window
--- @field title string
--- @field titleBarSize integer
--- @field rect Rect
--- @field style ui.Style
--- @field isOpen boolean
--- @field content string[]
--- @field cursorPos Vec2
--- @field regionAvail Vec2
--- @field scrollValue Vec2
--- @field components table
--- @field nextY integer
--- @field breakLine boolean
local Window = {}
local window_mt = {}
window_mt.__index = Window

--- @param title string
--- @param rect Rect
--- @param style ui.Style
--- @return ui.Window
function Window.create(title, rect, style)
    local w = {}
    w.title = title
    w.titleBarSize = 16
    w.rect = rect
    w.style = style
    w.isOpen = true
    w.content = {}
    w.cursorPos = Vec2.create(rect.x, rect.y + w.titleBarSize)
    w.regionAvail = Vec2.create(rect.w, rect.h)
    w.scrollValue = Vec2.create(0, 0)
    w.nextY = w.cursorPos[2]
    w.breakLine = true
    w.components = {}
    return setmetatable(w, window_mt)
end

--- @param comp
--- @param size {integer,integer}
function Window:addComponent(comp, size)
    local cursor = self.cursorPos
    --[[
    if cursor[1] + size[1] > (self.rect.x + self.rect.w) then
        cursor[1] = self.rect.x
        cursor[2] = cursor[2] + self.nextY
        self.nextY = size[2]
    else
        self.nextY = math.max(self.nextY, size[2])
    end
    ]]
    if self.breakLine then
        cursor[1] = self.rect.x
        cursor[2] = cursor[2] + size[2] + 4
        self.breakLine = false
    end
    table.insert(
        self.components,
        {
            id = comp.id,
            pos = { cursor[1], cursor[2] },
            size = size
        }
    )
    cursor[1] = cursor[1] + size[1] + 2
end

--- @param r Renderer
function Window:draw(r)
    r:setClipRect()
    local x0, y0, x1, y1 = 0, 0, 0, 0
    x0 = self.rect.x
    y0 = self.rect.y
    x1 = self.rect.x + self.rect.w
    y1 = self.rect.y + self.rect.h

    Frame.draw(r, x0, y0, x1, y0 + self.titleBarSize, self.style.window.titleBar)
    Frame.draw(r, x0, y0 + self.titleBarSize + 1, x1, y1, self.style.window)

    r:setClipRect(self.rect)
    r:setDrawColor(self.style.text.shadow)
    r:print(self.title, self.rect.x + 5, self.rect.y + 5)
    r:setDrawColor(self.style.text.fg)
    r:print(self.title, self.rect.x + 4, self.rect.y + 4)

    local fx = x1 - 2
    local fy = y1 - 2
    r:setDrawColor(self.style.text.shadow)
    r:fillTriangle({ fx, fy }, { fx - 8, fy }, { fx, fy - 8 })
    fx = fx - 1
    fy = fy - 1
    r:setDrawColor(self.style.text.fg)
    r:fillTriangle({ fx, fy }, { fx - 8, fy }, { fx, fy - 8 })
end

return Window
