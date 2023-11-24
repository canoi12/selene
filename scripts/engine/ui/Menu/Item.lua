--- @class ui.Menu.Item
--- @field label string
local Item = {}

local item_mt = {}
item_mt.__index = Item

--- @param label string
--- @return ui.Menu.Item
function Item.create(label)
    local item = {}
    item.label = label
    return setmetatable(item, item_mt)
end

function Item:draw(render, x0, y0, x1, y1)
end

return Item