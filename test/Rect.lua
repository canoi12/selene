--- @class Rect
--- @field x number
--- @field y number
--- @field w number
--- @field h number
local Rect = {}
local rect_mt = {
    __index = Rect
}

---Creates a new Rect
---@param x number
---@param y number
---@param w number
---@param h number
---@return Rect
function Rect.create(x, y, w, h)
    local r = {}
    r.x = x
    r.y = y
    r.w = w
    r.h = h

    return setmetatable(r, rect_mt)
end

---Return rect values
---@return number
---@return number
---@return number
---@return number
function Rect:getValues()
    return self.x, self.y, self.w, self.h
end

return Rect