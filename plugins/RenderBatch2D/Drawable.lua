local Drawable = {}
Drawable.__index = Drawable

function Drawable.create(texture)
    local dr = {}
    dr.texture = texture
    dr.flip = {false, false}
    return setmetatable(dr, Drawable)
end

function Drawable:draw(batch, x, y)
    batch.batch:push_sprite(self.texture, x, y, 0, 1, 1, nil, self.flip)
end

return Drawable