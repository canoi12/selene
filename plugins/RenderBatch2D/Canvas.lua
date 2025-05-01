local Canvas = {}
Canvas.__index = Canvas

function Canvas.create(batch, width, height, depth)
    local c = {}
    c.texture = batch.handle:create_texture2d(width, height, 'rgba', true)
    -- print(c.texture)
    if depth then
        c.depth = batch.handle:create_depth_texture(width, height)
    end
    local backend = batch.handle:get_backend()
    c.flip = {false, true}
    if backend ~= 'opengl' then c.flip[2] = false end
    c.target = batch.handle:create_render_target(c.texture, c.depth)
    return setmetatable(c, Canvas)
end

function Canvas:destroy(batch)
    if self.texture then batch.handle:destroy_texture(self.texture) end
    self.texture = nil
    if self.depth then batch.handle:destroy_texture(self.depth) end
    self.depth = nil
    if self.target then batch.handle:destroy_render_target(self.target) end
    self.target = nil
end

function Canvas:draw(batch, x, y)
    batch.batch:push_sprite(self.texture, x, y, 0, 1, 1, nil, self.flip)
end

return Canvas