local Color = require('graphics.Color')
--- @class ui.Frame
local Frame = {}

local defaultStyle = {
    bg = Color.gray,
    borders = {Color.white, Color.black}
}

--- @param r Renderer
--- @param x0 integer
--- @param y0 integer
--- @param x1 integer
--- @param y1 integer
--- @param style ui.FrameStyle | nil
function Frame.draw(r, x0, y0, x1, y1, style)
    style = style or defaultStyle
    r:setDrawColor(style.bg)
    r:fillRectangle(x0, y0, x1-x0, y1-y0)
    r:setDrawColor(style.borders[1])
    r:drawRectangle(x0, y0, x1-x0, y1-y0)
    r:setDrawColor(style.borders[2])
    r:drawLine(x0, y1, x1, y1)
    r:drawLine(x1, y1, x1, y0)
end

return Frame