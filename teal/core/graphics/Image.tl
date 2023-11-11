local gl = selene.gl
local Rect = require('core.Rect')
global enum ImageFilter
    "nearest"
    "linear"
end

local record Image
    handle: gl.Texture
    width: integer
    height: integer
    channels: integer
    filters: {ImageFilter}

    create: function(integer, integer, integer | nil, selene.Data | nil) : Image
    load: function(string) : Image

    metamethod __index: Image
    metamethod __gc: function(Image)
    
    setFilter: function(Image, ImageFilter, ImageFilter)
    getFilter: function(Image) : ImageFilter, ImageFilter

    getTexture: function(Image) : gl.Texture
    getUV: function(Image, Rect) : {number}
end

local image_mt : metatable<Image> = {}
image_mt.__index = Image
image_mt.__gc = function(s: Image)
    s.handle:destroy()
end

function Image.create(width: integer, height: integer, channels: integer | nil, data: selene.Data | nil) : Image
    local img : Image = {}
    local texture = gl.Texture.create()
    channels = channels or 4
    assert(channels > 0 and channels < 5)
    img.handle = texture
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

    return setmetatable(img, { __index = Image, __gc = function(i: Image) i.handle:destroy() end})
end

function Image.load(path: string) : Image
    local img : Image = {}
    local data, w, h, c = selene.image.loadData(path)
    local texture = gl.Texture.create()
    img.handle = texture
    img.width = w
    img.height = h
    img.channels = c
    gl.Texture.bind(gl.TEXTURE_2D, texture)
    gl.Texture.image2D(gl.TEXTURE_2D, gl.RGBA, w, h, gl.RGBA, gl.UNSIGNED_BYTE, data)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
    gl.Texture.bind(gl.TEXTURE_2D)
    data:free()
    return setmetatable(img, {__index = Image, __gc = function(i:Image) i.handle:destroy() end})
end

function Image:getTexture() : gl.Texture
    return self.handle
end

function Image:getUV(rect: Rect) : {number}
    local width = self.width
    local height = self.height

    rect = (rect or { x = 0, y = 0, w = width, h = height }) as Rect

    local uv = {}
    local uv_y = rect.y / height
    uv[1] = rect.x / width
    uv[2] = uv_y
    uv[3] = uv[1] + rect.w / width
    uv[4] = uv_y + rect.h / height
    return uv
end

return Image