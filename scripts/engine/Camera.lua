local Canvas = require('core.graphics.Canvas')
local Rect = require('core.Rect')
--- @class engine.Camera
--- @field canvas Canvas
local Camera = {}
local mt = {}
mt.__index = Camera

function Camera.create(data)
    local c = setmetatable({}, mt)
    c.scale = 1
    for _,field in ipairs(data.fieldInstances) do
        if field.__identifier == 'Scale' then
            c.scale = field.__value
        end
    end
    c.dest = Rect.create(0, 0, data.width * c.scale, data.height * c.scale)
    c.position = {data.px[1], data.px[2]}
    c.canvas = Canvas.create(data.width, data.height)
    return c
end

--- @param r core.Renderer
function Camera:begin(r)
    r:translate(-self.position[1], -self.position[2])
    r:setCanvas(self.canvas)
    r:clear()
end

--- @param r core.Renderer
function Camera:finish(r)
    r:setCanvas()
end

--- @param r core.Renderer
function Camera:onRender(r)
    r:identity()
    r:blit(self.canvas, nil, self.dest)
end

return Camera