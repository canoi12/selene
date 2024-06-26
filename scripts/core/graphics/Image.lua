local gl = selene.gl
--- @type Rect
local Rect = require('core.Rect')
local Drawable = require('core.graphics.Drawable')

--- @class Image : Drawable
--- @field channels integer
local Image = {}
local image_mt = {}
image_mt.__index = Image
image_mt.__gc = function(s)
    s.texture:destroy()
end

function Image.create(width, height, channels, data)
    local img = {}
    local texture = gl.Texture.create()
    channels = channels or 4
    assert(channels > 0 and channels < 5)
    img.texture = texture
    img.width = width
    img.height = height
    img.channels = channels

    gl.Texture.bind(gl.TEXTURE_2D, texture)
    gl.Texture.image2D(gl.TEXTURE_2D, gl.RGBA, width, height, gl.RGBA, gl.UNSIGNED_BYTE, data)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
    gl.Texture.bind(gl.TEXTURE_2D)

    return setmetatable(img, image_mt)
end

function Image.load(path)
    local img = {}
    local data, w, h, c = selene.image.loadData(path)
    local img = Image.create(w, h, c, data)
    data:free()
    return img
end

Image.getTexture = Drawable.getTexture
Image.getWidth = Drawable.getWidth
Image.getHeight = Drawable.getHeight

function Image:getUV(rect)
    local width = self.width
    local height = self.height

    self.

    rect = (rect or { x = 0, y = 0, w = width, h = height })

    local uv = {}
    local uv_y = rect.y / height
    uv[1] = rect.x / width
    uv[2] = uv_y
    uv[3] = uv[1] + rect.w / width
    uv[4] = uv_y + rect.h / height
    return uv
end

return Image
