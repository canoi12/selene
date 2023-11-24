local palette = require('editor.palette')
--- @type editor.Frame
local Frame = {}

local def_style = {
    bg = palette[4],
    border = {
        light = palette[2],
        dark = palette[5],
    }
}

--- Draw a Frame from rect
--- @param r Renderer
--- @param rect Rect
--- @param style table | nil
function Frame.draw(r, rect, style)
    style = style or def_style
    r:setDrawColor(style.bg)
    r:fillRectangle(rect.x, rect.y, rect.w, rect.h)
    r:setDrawColor(style.border.light)
    r:drawRectangle(rect.x, rect.y, rect.w, rect.h)
    r:setDrawColor(style.border.dark)
    r:drawLine(rect.x, rect.y + rect.h, rect.x + rect.w, rect.y + rect.h)
    r:drawLine(rect.x+rect.w, rect.y + rect.h, rect.x + rect.w, rect.y)
end

return Frame