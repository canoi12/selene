--- @class Color
local Color = {}

local color_mt = {}
color_mt.__index = Color

--- Create a rgb color
--- @param r integer
--- @param g integer
--- @param b integer
--- @return Color
function Color.rgb(r, g, b)
    local c = {}
    c[1] = r
    c[2] = g
    c[3] = b
    c[4] = 255
    return setmetatable(c, color_mt)
end

--- Create a rgba color
--- @param r integer
--- @param g integer
--- @param b integer
--- @param a integer
--- @return Color
function Color.rgba(r, g, b, a)
    local c = {}
    c[1] = r
    c[2] = g
    c[3] = b
    c[4] = a
    return setmetatable(c, color_mt)
end

--- Returns color in normalized float
---@return number, number, number, number
function Color:toFloat()
    local cf = {}
    for i,cc in ipairs(self) do
        cf[i] = cc / 255
    end
    return table.unpack(cf)
end

Color.red = Color.rgb(255, 0, 0)
Color.black = Color.rgb(0, 0, 0)
Color.white = Color.rgb(255, 255, 255)

return Color
