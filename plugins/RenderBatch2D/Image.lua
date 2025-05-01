local Image = {}
Image.__index = Image

function Image.create(batch, width, height, data)
    local i = {}
    i.texture = batch.handle:create_texture2d(width, height, 'rgba', data)
    i.src = nil
    return setmetatable(i, Image)
end

function Image.load(batch, path)
    local i = {}
    local img = image.from_file(path)
    i.texture = batch.handle:create_texture2d(img.width, img.height, 'rgba', img.data)
    return setmetatable(i, Image)
end

function Image:destroy(batch)
    if self.texture then
        batch.handle:destroy_texture(self.texture)
    end
end

function Image:draw(batch, x, y)
    batch.batch:push_sprite(self.texture, self.src, x, y)
end

return Image