--- @class Point
--- @field x number
--- @field y number
local Point = {}

local point_mt = {}
point_mt.__index = Point

--- Creates a new Point object
---@param x number
---@param y number
---@return Point
function Point.create(x, y)
    local p = {}
    p.x = x
    p.y = y
    return setmetatable(p, point_mt)
end

--- Get the values from x and y
---@return number
---@return number
function Point:getValues()
    return self.x, self.y
end

return Point