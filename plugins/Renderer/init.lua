local Renderer = {}

function Renderer.create()
    local r = setmetatable({}, {__index = Renderer})
    return r
end

function Renderer:begin_pass(data)
end

function Renderer:end_pass()
end

function Renderer:present()
end

function Renderer:draw_point(x, y)
end

function Renderer:draw_line(x0, y0, x1, y1)
end

function Renderer:draw_triangle(x0, y0, x1, y1, x2, y2)
end

function Renderer:draw_rectangle(x, y, w, h)
end

function Renderer:draw_circle(x, y, radius)
end

function Renderer:draw_sphere(x, y, r)
end

function Renderer:draw_cube(x, y, sx, sy)
end

function Renderer:draw_sprite(sprite, x, y)
end

return Renderer