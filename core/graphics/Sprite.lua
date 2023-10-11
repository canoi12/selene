local Drawable = require 'core.graphics.Drawable'
local Sprite = Drawable:extend('Sprite')

function Sprite:constructor(image, tile_size)
    self.image = image
    self.rects = {}
    self.speed = 8
    local width, height = image:get_size()
end

function Sprite:update(dt)
end

function Sprite:get_uv()
end

return Sprite