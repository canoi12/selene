local Rect = require('Rect')
local Frame = require('editor.Frame')
local style = require('editor.style')
--- @class editor.components.Menu.Item
--- @field name string
--- @field action function
--- @field rect Rect
--- @field style table
local Item = {}
local item_mt = {}
item_mt.__index = Item

function Item.create(name, action)
    local i = {}
    i.name = name
    i.action = action
    i.rect = Rect.create(0, 0, #name * 8 + 8, 15)
    i.style = style.button.normal
    return setmetatable(i, item_mt)
end

--- @param r Renderer
function Item:draw(r)
    Frame.draw(r, self.rect, self.style)
    r:setDrawColor(style.text.shadow)
    r:print(self.name, self.rect.x + 5, self.rect.y + 5)
    r:setDrawColor(style.text.fg)
    r:print(self.name, self.rect.x + 4, self.rect.y + 4)
end

--- @param ev event_types
function Item:onEvent(ev, btn, x, y)
    if ev == 'mouse motion' then
        if self.rect:intersect(x, y) then
            self.style = style.button.hover
        else
            self.style = style.button.normal
        end
    elseif ev == 'mouse button down' then
        if self.rect:intersect(x, y) then
            self.style = style.button.press
        end
    elseif ev == 'mouse button up' then
        self.style = style.button.normal
    end
end

return Item