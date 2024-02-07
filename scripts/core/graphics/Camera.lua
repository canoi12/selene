local Vec2 = require('math.Vec2')

--- @class Camera
--- @field position Vec2
--- @field offset Vec2
--- @field scale Vec2
--- @field angle number
local Camera = {}
local camera_mt = {
    __index = Camera
}

--- Creates a new camera
--- @return Camera
function Camera.create()
    local cam = {}
    cam.position = Vec2.create(0, 0)
    cam.offset = Vec2.create(0, 0)
    cam.scale = Vec2.create(1, 1)
    cam.angle = 0

    return setmetatable(cam, camera_mt)
end

--- @param matrix selene.linmath.Mat4
function Camera:applyInMatrix(matrix)
    matrix:translate(self.offset[1], self.offset[2])
    matrix:scale(self.scale[1], self.scale[2])
    matrix:rotate(self.angle)
    matrix:translate(-self.position[1], -self.position[2])
end

return Camera