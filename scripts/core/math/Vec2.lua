--- @class Vec2
local Vec2 = {}
local vec2_mt = {}
vec2_mt.__index = Vec2

vec2_mt.__add = function(self, other)
   return Vec2.create(self[1] + other[1], self[2] + other[2])
end

vec2_mt.__sub = function(self, other)
   return Vec2.create(self[1] - other[1], self[2] - other[2])
end

vec2_mt.__mul = function(a, b)
   return Vec2.create(a[1] * b, a[2] * b)
end

vec2_mt.__div = function(a, b)
   return Vec2.create(a[1] / b, a[2] / b)
end

function Vec2.create(x, y)
   local v = {}
   v[1] = x
   v[2] = y
   return setmetatable(v, vec2_mt)
end

function Vec2:add(v)
   self[1] = self[1] + v[1]
   self[2] = self[2] + v[2]
end

function Vec2:sub(v)
   self[1] = self[1] - v[1]
   self[2] = self[2] - v[2]
end

return Vec2
