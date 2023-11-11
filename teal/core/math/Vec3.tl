local record Vec3
    {number}

    create: function(number, number, number) : Vec3

    metamethod __add: function(Vec3, Vec3) : Vec3
    metamethod __sub: function(Vec3, Vec3) : Vec3
    metamethod __mul: function(Vec3, number) : Vec3
    metamethod __div: function(Vec3, number) : Vec3

    add: function(Vec3, Vec3)
    sub: function(Vec3, Vec3)
    cross: function(Vec3, Vec3) : number

    length: function(Vec3) : number
end

local function __add(a: Vec3, b: Vec3) : Vec3
    return Vec3.create(a[1] + b[1], a[2] + b[2], a[3] + b[3])
end

local function __sub(a: Vec3, b: Vec3) : Vec3
    return Vec3.create(a[1] - b[1], a[2] - b[2], a[3] - b[3])
end

local function __mul(a: Vec3, b: number) : Vec3
    return Vec3.create(a[1] * b, a[2] * b, a[3] * b)
end

local function __div(a: Vec3, b: number) : Vec3
    return Vec3.create(a[1] / b, a[2] / b, a[3] / b)
end

function Vec3.create(x: number, y: number, z: number) : Vec3
    local v : Vec3
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

function Vec3:add(v: Vec3)
    self[1] = self[1] + v[1]
    self[2] = self[2] + v[2]
    self[3] = self[3] + v[3]
end

function Vec3:sub(v: Vec3)
    self[1] = self[1] - v[1]
    self[2] = self[2] - v[2]
    self[3] = self[3] - v[3]
end

return Vec3