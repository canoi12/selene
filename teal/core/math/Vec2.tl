local record Vec2
    {number}

    create: function(number, number) : Vec2

    metamethod __add: function(Vec2, Vec2) : Vec2
    metamethod __sub: function(Vec2, Vec2) : Vec2
    metamethod __mul: function(Vec2, number) : Vec2
    metamethod __div: function(Vec2, number) : Vec2

    add: function(Vec2, Vec2)
    sub: function(Vec2, Vec2)
    cross: function(Vec2, Vec2) : number

    length: function(Vec2) : number
end

local function __add(self: Vec2, other: Vec2) : Vec2
    return Vec2.create(self[1] + other[1], self[2] + other[2])
end

local function __sub(self: Vec2, other: Vec2) : Vec2
    return Vec2.create(self[1] - other[1], self[2] - other[2])
end

local function __mul(a: Vec2, b: number) : Vec2
    return Vec2.create(a[1] * b, a[2] * b)
end

local function __div(a: Vec2, b: number) : Vec2
    return Vec2.create(a[1] / b, a[2] / b)
end

function Vec2.create(x: number, y: number) : Vec2
    local v : Vec2

    v[1] = x
    v[2] = y

    return setmetatable(v, {
        __index = Vec2,
        __add = __add,
        __sub = __sub,
        __mul = __mul,
        __div = __div
    })
end

function Vec2:add(v: Vec2)
    self[1] = self[1] + v[1]
    self[2] = self[2] + v[2]
end

function Vec2:sub(v: Vec2)
    self[1] = self[1] - v[1]
    self[2] = self[2] - v[2]
end

return Vec2