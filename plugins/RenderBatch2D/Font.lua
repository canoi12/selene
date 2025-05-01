local Font = {}
Font.__index = Font

function Font.create8x8(batch)
    local f = {}
    local font_8x8 = font.create8x8()
    f.texture = batch.handle:create_texture2d(font_8x8.width, font_8x8.height, font_8x8.format, font_8x8.data)
    f.glyphs = font_8x8.glyphs
    return setmetatable(f, Font)
end

function Font:destroy()
    if self.texture then self.texture:destroy() end
    self.texture = nil
    self.glyphs = nil
end

return Font