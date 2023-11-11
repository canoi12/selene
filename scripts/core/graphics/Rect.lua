local class = require 'core.class'
local Rect = class:extend("Rect")
local Rect = {}
local Rect_mt = {
  __index = Rect
}

function Rect.create(x, y, w, h)
  local r = {}
  r.x = x
  r.y = y
  r.w = w
  r.h = h
  return setmetatable(r, Rect_mt)
end

function Rect:constructor(x, y, w, h)
  self.x = x or 0
  self.y = y or 0
  self.w = w or 32
  self.h = h or 32
end

return Rect
