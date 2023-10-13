local Drawable = require 'core.graphics.Drawable'
local Image = require 'core.graphics.Image'
local filesystem = fs.
local Sprite = Drawable:extend('Sprite')

function Sprite:constructor(...)
    local args = {...}
    self.tile_size = 0
    local t1 = type(args[1])
    if t1 == "string" then
        local t2 = type(args[2])
        if #args > 1 then
            local image = Image(args[1])
            self.image = image
            self.tile_size = args[2]
        else
            -- load aseprite
        end
    elseif t1 == "table" then
        if t1:is("Image") then
            self.image = t1
        end
    end
    
    self.rects = {}
    self.speed = 8
    self.animations = {}
    self.animation = ""
    local width, height = image:get_size()
end

function Sprite:update(dt)
end

function Sprite:get_uv(r)
    local width = self.width
    local height = self.height
    rect = rect or { x = 0, y = 0, w = width, h = height }

    local uv = {}
    uv[1] = rect.x / width
    uv[2] = rect.y / height
    uv[3] = uv[1] + rect.w / width
    uv[4] = uv[2] + rect.h / height
    return uv
end

return Sprite