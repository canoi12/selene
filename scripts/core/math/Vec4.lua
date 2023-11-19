local Vec4 = {}

local function __add(a, b)
   return Vec4.create(a[1] + b[1], a[2] + b[2], a[3] + b[3], a[4] + b[4])
end

local function __sub(a, b)
   return Vec4.create(a[1] - b[1], a[2] - b[2], a[3] - b[3], a[4] - b[4])
end

local function __mul(a, b)
   return Vec4.create(a[1] * b, a[2] * b, a[3] * b, a[4] * b)
end

local function __div(a, b)
   return Vec4.create(a[1] / b, a[2] / b, a[3] / b, a[4] / b)
end

function Vec4.create(x, y, z, w)
   local v
   v[1] = x
   v[2] = y
   v[3] = z
   v[4] = w

   return setmetatable(v, {
      __index = Vec4,
      __add = __add,
      __sub = __sub,
      __mul = __mul,
      __div = __div,
   })
end

function Vec4:add(v)
   self[1] = self[1] + v[1]
   self[2] = self[2] + v[2]
   self[3] = self[3] + v[3]
   self[4] = self[4] + v[4]
end

function Vec4:sub(v)
   self[1] = self[1] - v[1]
   self[2] = self[2] - v[2]
   self[3] = self[3] - v[3]
   self[4] = self[4] - v[4]
end

return Vec4
