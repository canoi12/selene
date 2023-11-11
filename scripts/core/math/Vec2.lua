local Vec2 = {}
















local function __add(self, other)
   return Vec2.create(self[1] + other[1], self[2] + other[2])
end

local function __sub(self, other)
   return Vec2.create(self[1] - other[1], self[2] - other[2])
end

local function __mul(a, b)
   return Vec2.create(a[1] * b, a[2] * b)
end

local function __div(a, b)
   return Vec2.create(a[1] / b, a[2] / b)
end

function Vec2.create(x, y)
   local v

   v[1] = x
   v[2] = y

   return setmetatable(v, {
      __index = Vec2,
      __add = __add,
      __sub = __sub,
      __mul = __mul,
      __div = __div,
   })
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
