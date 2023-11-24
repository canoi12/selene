local Frame = require('editor.Frame')
local style = require('editor.style')
local Rect = require('Rect')
--- @class editor.Window
--- @field title string
--- @field rect Rect
--- @field topBar Rect
--- @field contentRegion Rect
--- @field resizing boolean
--- @field moving boolean
local Window = {}

function Window.create(title, rect)
    local w = {}
    w.title = title
    w.rect = rect
    w.topBar = Rect.create(rect.x, rect.y, rect.w, 16)
    w.contentRegion = Rect.create(rect.x, rect.y+16, rect.w, rect.h-32)

    return setmetatable(w, { __index = Window })
end

--- @param r Renderer
function Window:draw(r)
    -- Frame.draw(r, self.rect)

    self.topBar.x = self.rect.x
    self.topBar.y = self.rect.y
    self.topBar.w = self.rect.w

    self.contentRegion.x = self.rect.x
    self.contentRegion.y = self.rect.y + 17
    self.contentRegion.w = self.rect.w
    self.contentRegion.h = self.rect.h - 17

    Frame.draw(r, self.topBar, style.window.titleBar)
    Frame.draw(r, self.contentRegion, style.window)

    r:setDrawColor(style.text.shadow)
    r:print(self.title, self.rect.x + 5, self.rect.y + 5)
    r:setDrawColor(style.text.fg)
    r:print(self.title, self.rect.x + 4, self.rect.y + 4)

    local fx = self.rect.x + self.rect.w - 2
    local fy = self.rect.y + self.rect.h - 2
    -- Frame.draw(r, rr, style.button.press)
    -- r:fillRectangle(rr.x, rr.y, rr.w, rr.h)
    r:setDrawColor(style.text.shadow)
    r:fillTriangle({fx, fy}, {fx-8, fy}, {fx, fy-8})
    fx = fx - 1
    fy = fy - 1
    r:setDrawColor(style.text.fg)
    r:fillTriangle({fx, fy}, {fx-8, fy}, {fx, fy-8})
    -- if self.moving then
    --     r:print('moving', 0, 0)
    -- elseif self.resizing then
    --     r:print('resizing', 0, 0)
    -- else
    --     r:print('caraio!!')
    -- end
end

--- @param ev Event
function Window:onEvent(ev)
    local eventType = ev:getType()
    if eventType == 'mouse button down' then
        local which, btn, x, y = ev.handle:mouseButtonEvent()
        if self.topBar:intersect(x, y) then
            self.moving = true
        end
        local rr = {
            x = self.rect.x + (self.rect.w - 8),
            y = self.rect.y + (self.rect.h - 8),
            w = 8,
            h = 8
        }
        if Rect.intersect(rr, x, y) then
            self.resizing = true
        end
    elseif eventType == 'mouse button up' then
        self.moving = false
        self.resizing = false
    end

    if eventType == "mouse motion" then
        local which, state, x, y, xrel, yrel = ev.handle:mouseMotionEvent()
        if self.moving then
            self.rect.x = self.rect.x + xrel
            self.rect.y = self.rect.y + yrel
        elseif self.resizing then
            self.rect.w = self.rect.w + xrel
            self.rect.h = self.rect.h + yrel
        end
    end
end

return Window