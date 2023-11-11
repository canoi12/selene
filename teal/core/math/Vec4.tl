local record Vec4
    {number}

    create: function(x: number, y: number, z: number, w: number) : Vec4

    metamethod __index: Vec4
    metamethod __add: function(Vec4, Vec4) : Vec4
    metamethod __sub: function(Vec4, Vec4) : Vec4
    metamethod __mul: function(Vec4, number) : Vec4
    metamethod __div: function(Vec4, number) : Vec4

    add: function(Vec4, Vec4)
    sub: function(Vec4, Vec4)

    length: function(Vec4) : number
end

local function __add(a: Vec4, b: Vec4) : Vec4
    return Vec4.create(a[1] + b[1], a[2] + b[2], a[3] + b[3], a[4] + b[4])
end

local function __sub(a: Vec4, b: Vec4) : Vec4
    return Vec4.create(a[1] - b[1], a[2] - b[2], a[3] - b[3], a[4] - b[4])
end

local function __mul(a: Vec4, b: number) : Vec4
    return Vec4.create(a[1] * b, a[2] * b, a[3] * b, a[4] * b)
end

local function __div(a: Vec4, b: number) : Vec4
    return Vec4.create(a[1] / b, a[2] / b, a[3] / b, a[4] / b)
end

function Vec4.create(x: number, y: number, z: number, w: number) : Vec4
    local v : Vec4
    v[1] = x
    v[2] = y
    v[3] = z
    v[4] = w

    return setmetatable(v, {
        __index = Vec4,
        __add = __add,
        __sub = __sub,
        __mul = __mul,
        __div = __div
    })
end

function Vec4:add(v: Vec4)
    self[1] = self[1] + v[1]
    self[2] = self[2] + v[2]
    self[3] = self[3] + v[3]
    self[4] = self[4] + v[4]
end

function Vec4:sub(v: Vec4)
    self[1] = self[1] - v[1]
    self[2] = self[2] - v[2]
    self[3] = self[3] - v[3]
    self[4] = self[4] - v[4]
end

return Vec4