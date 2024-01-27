local Point = require 'Point'

local p = Point.New(0, 32)

print(p, p.x, p.y)

local enum level
    "empty"
    "low"
    "medium"
    "hight"
end

print('teste')
print('level', level)

for k,v in pairs(level) do
    print(k, v)
end

function queiss(q: level)
    print(q)
end

queiss("medium")