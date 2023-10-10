local class = require 'core.class'
local Rect = class:extend('Rect')

function Rect:constructor(x, y, w, h)
  self.x = x or 0
  self.y = y or 0
  self.width = w or 32
  self.height = h or 32
end

return Rect
