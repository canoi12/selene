local record Rect
    x: number
    y: number
    w: number
    h: number

    create: function(number, number, number, number) : Rect

    metamethod __index: Rect

    getValues: function(Rect) : number, number, number, number
end

local rect_mt : metatable<Rect> = {}
rect_mt.__index = Rect

function Rect.create(x: number, y: number, w: number, h: number) : Rect
    local r : Rect = {}
    
    r.x = x
    r.y = y
    r.w = w
    r.h = h

    return setmetatable(r, rect_mt)
end

function Rect:getValues() : number, number, number, number
    return self.x, self.y, self.w, self.h
end

return Rect