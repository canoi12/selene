local class = require 'core.class'
local Point = class:extend('Point')

function Point:constructor(x, y)
  self.x = x or 0
  self.y = y or 0
end

return Point
