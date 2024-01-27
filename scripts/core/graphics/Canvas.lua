local gl = selene.gl
--- @type Rect
local Rect = require 'core.Rect'

--- @class Canvas : Drawable
--- @field handle gl.Framebuffer
--- @field texture gl.Texture | nil
--- @field width integer
--- @field height integer
--- @field channels integer
local Canvas = {}
local canvas_mt = {
    __index = Canvas,
    __gc = function(c)
        c.handle:destroy()
        c.texture:destroy()
    end
}

function Canvas.create(width, height)
    local canvas = {}

    canvas.width = width
    canvas.height = height
    canvas.channels = 4
    canvas.texture = gl.Texture.create()
    gl.Texture.bind(gl.TEXTURE_2D, canvas.texture)
    gl.Texture.image2D(gl.TEXTURE_2D, gl.RGBA, width, height, gl.RGBA, gl.UNSIGNED_BYTE)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE)
    gl.Texture.parameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE)
    gl.Texture.bind(gl.TEXTURE_2D)

    canvas.handle = gl.Framebuffer.create()
    gl.Framebuffer.bind(gl.FRAMEBUFFER, canvas.handle)
    gl.Framebuffer.attachTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, canvas.texture)
    gl.Framebuffer.bind(gl.FRAMEBUFFER)

    return setmetatable(canvas, canvas_mt)
end

function Canvas.default()

end

function Canvas:getTexture()
    return self.texture
end

function Canvas:getUV(rect)
    local width = self.width
    local height = self.height

    rect = (rect or { x = 0, y = 0, w = width, h = height })

    local uv = {}
    local uv_y = rect.y / height
    uv[1] = rect.x / width
    uv[2] = 1 - uv_y
    uv[3] = uv[1] + rect.w / width
    uv[4] = 1 - (uv_y + rect.h / height)
    return uv
end

function Canvas:getWidth() return self.width end
function Canvas:getHeight() return self.height end

return Canvas
