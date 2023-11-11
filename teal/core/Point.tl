local record Point
    x: number
    y: number

    create: function(x: number, y: number) : Point

    metamethod __index: Point

    getValues: function(Point) : number, number
end

function Point.create(x: number, y: number) : Point
    local p : Point
    p.x = x
    p.y = y
    return setmetatable(p, { __index = Point})
end

function Point:getValues() : number, number
    return self.x, self.y
end

return Point