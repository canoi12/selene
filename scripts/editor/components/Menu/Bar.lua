local Rect = require('Rect')
local Frame = require('editor.Frame')
--- @class editor.components.Menu.Bar
--- @field rect Rect
--- @field items editor.components.Menu.Item[]
local Bar = {}

local menu_mt = {}
menu_mt.__index = Bar

--- @return editor.components.Menu.Bar
function Bar.create()
    local menu = {}

    menu.rect = Rect.create(0, 0, 640, 16)
    menu.items = {}

    return setmetatable(menu, menu_mt)
end

function Bar:update(dt)
    local x = self.rect.x + 1
    for i,item in ipairs(self.items) do
        item.rect.x = x
        x = x + item.rect.w + 1
    end
end

--- @param r Renderer
function Bar:draw(r)
    Frame.draw(r, self.rect)
    for i,item in ipairs(self.items) do
        item:draw(r)
    end
end

--- @param ev Event
function Bar:onEvent(ev)
    local tp = ev:getType()
    if tp == 'mouse motion' or
        tp == 'mouse button down' or
        tp == 'mouse button up' then
        local which, state, x, y = ev.handle:mouseMotionEvent()
        for i,item in ipairs(self.items) do
            item:onEvent(tp, state, x, y)
        end
    end
end

return Bar