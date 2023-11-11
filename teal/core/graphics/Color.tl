local record Color
    {integer}

    red: Color
    white: Color
    black: Color
    
    toFloat: function(Color) : number, number, number, number

    rgb: function(integer, integer, integer) : Color
    rgba: function(integer, integer, integer, integer) : Color
end

local color_mt : metatable<Color> = {}
color_mt.__index = Color


function Color.rgb(r: integer, g: integer, b: integer) : Color
    local c : Color = {}
    c[1] = r
    c[2] = g
    c[3] = b
    c[4] = 255
    return setmetatable(c, color_mt)
end

function Color.rgba(r: integer, g: integer, b: integer, a: integer) : Color
    local c : Color = {}
    c[1] = r
    c[2] = g
    c[3] = b
    c[4] = a
    return setmetatable(c, color_mt)
end

function Color:toFloat() : number, number, number, number
    local cf : {number} = {}
    for i,cc in ipairs(self) do
        cf[i] = cc / 255
    end
    return table.unpack(cf)
end

Color.red = Color.rgb(255, 0, 0)
Color.black = Color.rgb(0, 0, 0)
Color.white = Color.rgb(255, 255, 255)

return Color
