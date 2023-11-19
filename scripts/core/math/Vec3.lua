local Vec3 = {}

local function __add(a, b)
   return Vec3.create(a[1] + b[1], a[2] + b[2], a[3] + b[3])
end

local function __sub(a, b)
   return Vec3.create(a[1] - b[1], a[2] - b[2], a[3] - b[3])
end

local function __mul(a, b)
   return Vec3.create(a[1] * b, a[2] * b, a[3] * b)
end

local function __div(a, b)
   return Vec3.create(a[1] / b, a[2] / b, a[3] / b)
end

function Vec3.create(x, y, z)
   local v
   v[1] = x
   v[2] = y
   v[3] = z

   return setmetatable(v, {
      __index = Vec3,
      __add = __add,
      __sub = __sub,
      __mul = __mul,
      __div = __div,
   })
end

function Vec3:add(v)
   self[1] = self[1] + v[1]
   self[2] = self[2] + v[2]
   self[3] = self[3] + v[3]
end

function Vec3:sub(v)
   self[1] = self[1] - v[1]
   self[2] = self[2] - v[2]
   self[3] = self[3] - v[3]
end

return Vec3
