local Rect = require('Rect')
local Frame = require('editor.Frame')
--- @class editor.MenuBar
--- @field rect Rect
--- @field items table
local MenuBar = {}

local menu_mt = {}
menu_mt.__index = MenuBar

--- @return editor.MenuBar
function MenuBar.create()
    local menu = {}

    menu.rect = Rect.create(0, 0, 640, 16)
    menu.items = {}

    return setmetatable(menu, menu_mt)
end

--- @param r Renderer
function MenuBar:draw(r)
    Frame.draw(r, self.rect)
end


return MenuBar