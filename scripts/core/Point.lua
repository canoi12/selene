local Point = {}










function Point.create(x, y)
   local p
   p.x = x
   p.y = y
   return setmetatable(p, { __index = Point })
end

function Point:getValues()
   return self.x, self.y
end

return Point
